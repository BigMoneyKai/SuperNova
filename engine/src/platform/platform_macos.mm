#import "platform.h"
#if SNPLATFORM_MAC

#import "core/logger.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <stdlib.h>
#import <string.h>
#import <time.h>
#import <unistd.h>

typedef struct internal_state {
  NSWindow *window;
  NSAutoreleasePool *pool;
  id app_delegate;
} internal_state;

@interface PlatformAppDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic, assign) platform_state *plat_state;
@end

@implementation PlatformAppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}
@end

b8 platform_startup(platform_state *plat_state, const char *application_name,
                    i32 x, i32 y, i32 width, i32 height) {
  plat_state->internal_state = malloc(sizeof(internal_state));
  internal_state *state = (internal_state *)plat_state->internal_state;

  @autoreleasepool {
    state->pool = [[NSAutoreleasePool alloc] init];

    NSApplication *app = [NSApplication sharedApplication];

    PlatformAppDelegate *delegate = [[PlatformAppDelegate alloc] init];
    delegate.plat_state = plat_state;
    state->app_delegate = delegate;
    [app setDelegate:delegate];

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                       NSWindowStyleMaskResizable;

    NSRect frame = NSMakeRect(x, y, width, height);
    state->window = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:style
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];

    NSString *title = [NSString stringWithUTF8String:application_name];
    [state->window setTitle:title];
    [state->window makeKeyAndOrderFront:nil];

    [app activateIgnoringOtherApps:YES];
  }

  return SN_TRUE;
}

void platform_shutdown(platform_state *plat_state) {
  internal_state *state = (internal_state *)plat_state->internal_state;
  @autoreleasepool {
    if (state->window) {
      [state->window close];
    }
    if (state->pool) {
      [state->pool drain];
    }
    free(state);
    plat_state->internal_state = NULL;
  }
}

b8 platform_pump_messages(platform_state *plat_state) {
  internal_state *state = (internal_state *)plat_state->internal_state;
  @autoreleasepool {
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
      [NSApp sendEvent:event];

      if (event.type == NSEventTypeApplicationDefined) {
        return SN_FALSE;
      }
    }
  }

  return SN_TRUE;
}

void* platform_allocate(u64 size) {
    return malloc(size);
}

void* platform_allocate_aligned(u64 size, u64 alignment) {
    void* ptr = NULL;
    if(posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

void platform_free(void *block, b8 aligned) { free(block); }

void *platform_zero_memory(void *block, u64 size) {
  return memset(block, 0, size);
}

void *platform_copy_memory(void *dest, const void *src, u64 size) {
  return memcpy(dest, src, size);
}

void *platform_set_memory(void *dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char *msg, u8 color) {
  INFO("\033[%dm%s\033[0m", color, msg);
}

void platform_console_write_error(const char *msg, u8 color) {
  ERROR("\033[%dm%s\033[0m", color, msg);
}

// --- Time ---
f64 platform_get_absolute_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (f64)ts.tv_sec + (f64)ts.tv_nsec * 1e-9;
}

void platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000ULL;
  nanosleep(&ts, 0);
#else
  if (ms >= 1000)
    sleep(ms / 1000);
  usleep((ms % 1000) * 1000);
#endif
}

#endif
