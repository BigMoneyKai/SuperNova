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
typedef uint32_t b32;
typedef uint8_t b8;

// address types
typedef uintptr_t addr;

// properly define static assertion
#if defined(__clang__) || defined(__GNUC__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// type assertions
STATIC_ASSERT(sizeof(u8) == 1, "Exptypeofected u8 to be 1 byte.");
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

STATIC_ASSERT(sizeof(addr) == sizeof(void*), "Expected addr to match pointer size.");

// define boolean values
#define SN_TRUE 1
#define SN_FALSE 0

// platform compatibility
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define SN_PLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows."
    #endif

#elif defined(__APPLE__)
    #define SN_PLATFORM_APPLE 1
    #include <TargetConditional.h>
    #if TARGET_OS_IPHONE
        #define SN_PLATFORM_IOS 1
    #elif TARGET_OS_MAC
        #define SN_PLATFORM_MAC 1
    #else
        #error "Other unsupported operating system."
    #endif

#elif defined(__linux__) || defined(__gnu_linux__)
    #define SN_PLATFORM_LINUX 1
    #if defined(__ANDROID__)
        #define SN_PLATFORM_ANDROID 1
    #endif

#elif defined(__unix__)
    #define SN_PLATFORM_UNIX 1
    #if defined(_POSIX_VERSION)
        #define SN_PLATFORM_POSIX 1
    #endif

#else
    #error "Unknown/Unsupported operating system."
#endif

// .dll/.so
#if defined(_MSC_VER)
    #if defined(SN_EXPORT)
        #define SNAPI __declspec(dllexport)
    #elif defined(SN_IMPORT)
        #define SNAPI __declspec(dllimport)
    #else
        #define SNAPI
    #endif
#else
    #define SNAPI __attribute__((visibility("default")))
#endif

// cross-platform compatibility
#if defined(_MSC_VER)
    #define SN_RESTRICT __restrict
    #define SN_ALIGN(x) __declspec(align(x))
    #include <intrin.h>
    #define debug_break() __debugbreak()
    #define SN_INLINE __forceinline
    #define SN_COMPILER_BARRIER() _ReadWriteBarrier()
    #define SN_CPU_BARRIER() MemoryBarrier()
#else
    #define SN_RESTRICT restrict
    #define SN_ALIGN(x) __attribute__((aligned(x)))
    #define debug_break() __builtin_debugtrap()
    #if defined(SN_DEBUG)
        #define SN_INLINE inline
    #else
        #define SN_INLINE static inline __attribute__((always_inline))
    #endif
    #define SN_COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
    #define SN_CPU_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#endif

#define SN_MEM_BARRIER() \
    do { SN_COMPILER_BARRIER(); SN_CPU_BARRIER(); } while(0)

// compiler detection
#if defined(_MSC_VER)
    #define SN_COMPILER_MSVC 1
#elif defined(__clang__)
    #define SN_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define SN_COMPILER_GCC 1
#endif
