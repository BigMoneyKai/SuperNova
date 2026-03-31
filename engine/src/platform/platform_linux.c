#define _POSIX_C_SOURCE 200809L

#include "platform.h"
#if SNPLATFORM_LINUX

#include "core/logger.h"

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct internal_state {
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
} internal_state;

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height
) {
    plat_state->internal_state = malloc(sizeof(internal_state));
    if(!plat_state->internal_state) {
        ERROR("Failed to allocate Linux platform state.");
        return SN_FALSE;
    }
    internal_state* state = (internal_state*)plat_state->internal_state;

    state->connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(state->connection)) {
        ERROR("Failed to connect to X server.\n");
        free(state);
        plat_state->internal_state = NULL;
        return SN_FALSE;
    }

    const xcb_setup_t* setup = xcb_get_setup(state->connection);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    state->screen = it.data;

    state->window = xcb_generate_id(state->connection);

    u32 value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 value_list[] = {
        state->screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE |
        XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE |
        XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY
    };

    xcb_create_window(
        state->connection,
        XCB_COPY_FROM_PARENT,
        state->window,
        state->screen->root,
        x, y,
        width, height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        state->screen->root_visual,
        value_mask,
        value_list
    );

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(application_name),
        application_name
    );

    xcb_intern_atom_cookie_t wm_delete_cookie =
        xcb_intern_atom(state->connection, 0, 16, "WM_DELETE_WINDOW");

    xcb_intern_atom_cookie_t wm_protocols_cookie =
        xcb_intern_atom(state->connection, 0, 12, "WM_PROTOCOLS");

    xcb_intern_atom_reply_t* wm_delete_reply =
        xcb_intern_atom_reply(state->connection, wm_delete_cookie, NULL);

    xcb_intern_atom_reply_t* wm_protocols_reply =
        xcb_intern_atom_reply(state->connection, wm_protocols_cookie, NULL);

    state->wm_delete_win = wm_delete_reply->atom;
    state->wm_protocols = wm_protocols_reply->atom;

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        state->wm_protocols,
        XCB_ATOM_ATOM,
        32,
        1,
        &state->wm_delete_win
    );

    free(wm_delete_reply);
    free(wm_protocols_reply);

    xcb_map_window(state->connection, state->window);

    xcb_flush(state->connection);

    return SN_TRUE;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    if (!state) {
        return;
    }
    if (state->connection) {
        xcb_destroy_window(state->connection, state->window);
        xcb_disconnect(state->connection);
    }
    free(state);
    plat_state->internal_state = NULL;
}

b8 platform_pump_messages(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    xcb_generic_event_t* event;

    while ((event = xcb_wait_for_event(state->connection))) {
        if(!event) {
            return SN_FALSE;
        }
        u8 type = event->response_type & ~0x80;

        switch (type) {
            case XCB_CLIENT_MESSAGE: {
                xcb_client_message_event_t* cm =
                    (xcb_client_message_event_t*)event;

                if (cm->type == state->wm_protocols && cm->data.data32[0] == state->wm_delete_win) {
                    free(event);
                    return SN_FALSE;
                }
            } break;

            case XCB_KEY_PRESS:
                break;

            case XCB_BUTTON_PRESS:
                break;

            case XCB_MOTION_NOTIFY:
                break;

            case XCB_CONFIGURE_NOTIFY:
                break;
        }

        free(event);
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

void platform_free(void* block, b8 aligned) {
    (void)aligned;
    free(block);
}

void* platform_zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* src, u64 size) {
    return memcpy(dest, src, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platform_console_write(const char* msg, u8 color) {
    INFO("\033[%dm%s\033[0m", color, msg);
}

void platform_console_write_error(const char* msg, u8 color) {
    ERROR("\033[%dm%s\033[0m", color, msg);
}

f64 platform_get_absolute_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (f64)ts.tv_sec + (f64)ts.tv_nsec * 1e-9;
}

void platform_sleep(u64 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000ULL;
    nanosleep(&ts, 0);
}

#endif
