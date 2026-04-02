#pragma once

#include "defines.h"
// if no asserts, comment the line below
#define SNASSERTIONS_ENABLED 1

#if SNASSERTIONS_ENABLED
void report_assertion_message(const char* file_path, i32 line, const char* msg, ...);

#define ASSERT(cond, msg, ...)  \
    do {                        \
        if(cond) {              \
        } else {                \
            const char* assert_msg = "Assertion failed: ("#cond")"msg;          \
            report_assertion_message(__FILE__, __LINE__, assert_msg, ##__VA_ARGS__);\
            debug_break();      \
        }                       \
    } while(0)
#else
    #define ASSERT(cond, msg, ...) (void)(cond)

#endif
