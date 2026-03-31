#include <core/logger.h>
#include <core/asserts.h>
#include <core/memory.h>
#include <platform/platform.h>
#include <defines.h>

#include <stdlib.h>

int main(int argc, char** argv) {
    memsys_state app_state;

    if(!init_logging()) {
        return 1;
    }

    if(!snminit(0, &app_state)) {
        quit_logging();
        return 1;
    }

    char* ptr1 = snmalloc(32, MEM_TAG_ARRAY);
    char* ptr2 = snmalloc(32, MEM_TAG_DARRAY);
    char* ptr3 = snmalloc(32, MEM_TAG_SCENE);

    print_mstats();
    snmfree(ptr1, 32, MEM_TAG_ARRAY);
    snmfree(ptr2, 32, MEM_TAG_DARRAY);
    snmfree(ptr3, 32, MEM_TAG_SCENE);

    platform_state state = {0};
    if(platform_startup(&state, "Supernova Engine", 100, 100, 1280, 720)) {
        platform_shutdown(&state);
    } else {
        WARN("platform startup failed; memory smoke test still passed");
    }

    snmquit(&app_state);
    quit_logging();
    return 0;
}
