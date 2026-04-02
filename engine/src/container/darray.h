#pragma once

#include "core/logger.h"
#include "defines.h"

typedef struct darray {
    void* data;
    u64 length;
    u64 capacity;
    u64 stride;
} darray;

SNAPI void _darray_create(darray* arr, u64 capacity, u64 stride);
SNAPI void _darray_destroy(darray* arr);
SNAPI darray* _darray_resize(darray* arr);
SNAPI darray* _darray_push(darray* arr, const void* v_ptr);
SNAPI darray* _darray_insert(darray* arr, u64 idx, const void* v_ptr);
SNAPI darray* _darray_pop(darray* arr, void* dest);
SNAPI darray* _darray_pop_at(darray* arr, u64 idx, void* dest);

#define DARRAY_DEFAULT_CAPACITY 16
#define DARRAY_RESIZE_FACTOR 2

// External interface
#define darray_create(darr, type)\
    do {\
        _darray_create((darr), DARRAY_DEFAULT_CAPACITY, sizeof(type));\
    } while(0)

#define darray_destroy(darr)\
    do {\
        _darray_destroy((darr));\
    } while(0)

#define darray_resize(darr)\
    do {\
        (void)_darray_resize((darr));\
    } while(0)

#define darray_push(darr, val)\
    do {\
        typeof(val) temp = val;\
        (void)_darray_push((darr), (&temp));\
    } while(0)


#define darray_insert(darr, idx, val)\
    do {\
        typeof(val) temp = val;\
        (void)_darray_insert((darr), (idx), (&temp));\
    } while(0)


#define darray_pop(darr, dest)\
    do {\
        (void)_darray_pop((darr), (dest));\
    } while(0)

#define darray_pop_at(darr, idx, dest)\
    do {\
        (void)_darray_pop_at((darr), (idx), (dest));\
    } while(0)
