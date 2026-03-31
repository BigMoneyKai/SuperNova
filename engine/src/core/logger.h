#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED    1
#define LOG_INFO_ENABLED    1
#define LOG_DEBUG_ENABLED   1
#define LOG_TRACE_ENABLED   1

#if SNRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
} log_level;

b8 init_logging();
void quit_logging();

void log_output(log_level level, const char* file_path, i32 line, const char* msg, ...);

// fatal
#define FATAL(msg, ...) log_output(LOG_LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__);

// error
#define ERROR(msg, ...) log_output(LOG_LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__);

// warning
#if LOG_WARN_ENABLED == 1
#define WARN(msg, ...)  log_output(LOG_LEVEL_WARN, __FILE__, __LINE__, msg, ##__VA_ARGS__);
#else
#define WARN(msg, ...)
#endif

// info
#if LOG_INFO_ENABLED == 1
#define INFO(msg, ...)  log_output(LOG_LEVEL_INFO, __FILE__, __LINE__, msg, ##__VA_ARGS__);
#else
#define INFO(msg, ...)
#endif

// debug
#if LOG_DEBUG_ENABLED == 1
#define DEBUG(msg, ...)  log_output(LOG_LEVEL_DEBUG, __FILE__, __LINE__, msg, ##__VA_ARGS__);
#else
#define DEBUG(msg, ...)
#endif

// trace
#if LOG_TRACE_ENABLED == 1
#define TRACE(msg, ...)  log_output(LOG_LEVEL_TRACE, __FILE__, __LINE__, msg, ##__VA_ARGS__);
#else
#define TRACE(msg, ...)
#endif
