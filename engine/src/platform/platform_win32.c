#include "platform.h"
#if SNPLATFORM_WINDOWS

#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>

typedef struct internal_state {
    HINSTANCE h_instance;
    HWND hwnd;
    b8 is_running;
} internal_state;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, w_param, l_param);
}

b8 platform_startup(
    platform_state* plat_state,
    const char* application_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height
) {
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)plat_state->internal_state;

    state->h_instance = GetModuleHandle(0);

    WNDCLASSA wc = {0};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.hInstance = state->h_instance;
    wc.lpszClassName = "engine_window_class";

    if(!RegisterClassA(&wc)) {
        return SN_FALSE;
    }

    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        application_name,
        WS_OVERLAPPEDWINDOW,
        x, y, width, height,
        0, 0,
        state->h_instance,
        0
    );

    if(!hwnd) return SN_FALSE;

    state->hwnd = hwnd;
    state->is_running = SN_TRUE;

    ShowWindow(hwnd, SW_SHOW);
    return SN_TRUE;
}

void platform_shutdown(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    if(state->hwnd) {
        DestroyWindow(state->hwnd);
    }
}

b8 platform_pump_messages(platform_state* plat_state) {
    internal_state* state = (internal_state*)plat_state->internal_state;
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            state->is_running = SN_FALSE;
            return SN_FALSE;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return SN_TRUE;
}

void* platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void* block, b8 aligned) {
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
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(console, color);
    OutputDebugStringA(msg);
    INFO("%s", msg);
}

void platform_console_write_error(const char* msg, u8 color) {
    HANDLE console = GetStdHandle(STD_ERROR_HANDLE);
    SetConsoleTextAttribute(console, color);
    OutputDebugStringA(msg);
    ERROR("\033[%sm%s\033[0m", color, msg);
}

f64 platform_get_absolute_time() {
    static LARGE_INTEGER freq;
    static b8 initialized = SN_FALSE;

    if(!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = SN_TRUE;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (f64)now.QuadPart / (f64)freq.QuadPart;
}

void platform_sleep(u64 ms) {
    if(ms > 0xFFFFFFFF) {
        ms = 0xFFFFFFFF; // sleep clamp
    }
    Sleep((DWORD)ms);
}

#endif
