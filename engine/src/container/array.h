#pragma once

#include "core/logger.h"
#include "defines.h"

typedef struct array {
    void* data;
    u64 length;
    u64 capacity;
    u64 stride;
} array;

SNAPI void _array_create(array* arr, u64 capacity, u64 stride);
SNAPI void _array_destroy(array* arr);
SNAPI array* _array_push(array* arr, const void* v_ptr);
SNAPI array* _array_insert(array* arr, u64 idx, const void* v_ptr);
SNAPI array* _array_pop(array* arr, void* dest);
SNAPI array* _array_pop_at(array* arr, u64 idx, void* dest);

#define ARRAY_DEFAULT_CAPACITY 16

// External interface
#define array_create(darr, type)\
    do {\
        _array_create((darr), ARRAY_DEFAULT_CAPACITY, sizeof(type));\
    } while(0)

#define array_destroy(darr)\
    do {\
        _array_destroy((darr));\
    } while(0)

#define array_push(darr, val)\
    do {\
        typeof(val) temp = val;\
        (void)_array_push((darr), (&temp));\
    } while(0)


#define array_insert(darr, idx, val)\
    do {\
        typeof(val) temp = val;\
        (void)_array_insert((darr), (idx), (&temp));\
    } while(0)


#define array_pop(darr, dest)\
    do {\
        (void)_array_pop((darr), (dest));\
    } while(0)

#define array_pop_at(darr, idx, dest)\
    do {\
        (void)_array_pop_at((darr), (idx), (dest));\
    } while(0)
