#include <core/logger.h>
#include <core/asserts.h>
#include <platform/platform.h>
#include <defines.h>

#include <stdlib.h>

int main(int argc, char** argv) {
    platform_state state;
    INFO("platform state initialized.");
    if(platform_startup(&state, "Supernova Engine", 100, 100, 1280, 720)) {
        while(platform_pump_messages(&state)) {

        }
    }
    platform_shutdown(&state);

    return 0;
}
