#include "asserts.h"
#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

#if SNASSERTIONS_ENABLED
void report_assertion_message(const char* file_path, i32 line, const char* msg, ...) {
    char out_msg[32000];
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(out_msg, sizeof(out_msg), msg, arg_ptr);
    log_output(LOG_LEVEL_FATAL, file_path, line, "%s", out_msg);
    va_end(arg_ptr);
}
#endif
