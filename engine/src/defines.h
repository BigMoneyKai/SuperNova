#pragma once

#include <stdint.h>

// unsigned int
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// signed int
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// float point types
typedef float f32;
typedef double f64;

// boolean types
typedef int b32;
typedef char b8;

// properly define static assertion
#if defined(__clang__) || defined(__GNUC__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// type assertions
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 byte.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 byte.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 byte.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 byte.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 byte.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 byte.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 byte.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 byte.");

STATIC_ASSERT(sizeof(b32) == 4, "Expected b32 to be 4 byte.");
STATIC_ASSERT(sizeof(b8) == 1, "Expected b8 to be 1 byte.");

// define boolean values
#define SN_TRUE 1
#define SN_FALSE 0

// platform compatibility
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define SNPLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows."
    #endif

#elif defined(__APPLE__)
    #define SNPLATFORM_APPLE 1
    #include <TargetConditional.h>
    #if TARGET_OS_IPHONE
        #define SNPLATFORM_IOS 1
    #elif TARGET_OS_MAC
        #define SNPLATFORM_MAC 1
    #else
        #error "Other unsupported operating system."
    #endif

#elif defined(__linux__) || defined(__gnu_linux__)
    #define SNPLATFORM_LINUX 1
    #if defined(__ANDROID__)
        #define SNPLATFORM_ANDROID 1
    #endif

#elif defined(__unix__)
    #define SNPLATFORM_UNIX 1
    #if defined(_POSIX_VERSION)
        #define SNPLATFORM_POSIX 1
    #endif

#else
    #error "Unknown/Unsupported operating system."
#endif

#ifdef SNEXPORT
// export
    #ifdef _MSC_VER // msvc
        #define SNAPI __declspec(dllexport)
    #else // gcc/clang
        #define SNAPI __attribute__((visibility("default")))
    #endif
#else
// import
    #ifdef _MSC_VER // msvc
        #define SNAPI __declspec(dllimport)
    #else // gcc/clang
        #define SNAPI
    #endif
#endif
