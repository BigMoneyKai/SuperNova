#include "core/logger.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 init_logging() {
    // TODO: creat a log file
    return SN_TRUE;
}

void quit_logging() {
    // TODO: clean up logging/write queued entries
}

SNAPI void log_output(log_level level, const char* file_path, i32 line, const char* msg, ...) {
    const char* level_strs[6] = {"[fatal]: ", "[error]: ", "[warning]: ", "[info]: ", "[debug]: ", "[trace]: "};
    b8 is_error = level < 2;

    char org_msg[32000];
    memset(org_msg, 0, sizeof(org_msg));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(org_msg, 32000, msg, arg_ptr);
    va_end(arg_ptr);

    // TODO: platform-specific output
    char out_msg[32000];
    sprintf(out_msg, "%s%s\n", level_strs[level], org_msg);
    if(level == LOG_LEVEL_INFO) {
        printf("%s", out_msg);
    } else {
        printf("%s at %s:%d", out_msg, file_path, line);
    }
}
