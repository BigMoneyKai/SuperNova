// allocation-determined array (adarray)

#pragma once

#include "core/logger.h"
#include "defines.h"

typedef struct adarray {
    void* data;
    u64 length;
    u64 capacity;
    u64 stride;
} adarray;

SNAPI void _adarray_create(adarray* arr, u64 capacity, u64 stride);
SNAPI void _adarray_destroy(adarray* arr);
SNAPI adarray* _adarray_resize(adarray* arr);
SNAPI adarray* _adarray_push(adarray* arr, const void* v_ptr);
SNAPI adarray* _adarray_insert(adarray* arr, u64 idx, const void* v_ptr);
SNAPI adarray* _adarray_pop(adarray* arr, void* dest);
SNAPI adarray* _adarray_pop_at(adarray* arr, u64 idx, void* dest);

#define ADARRAY_DEFAULT_CAPACITY 16
#define ADARRAY_RESIZE_FACTOR 2

// External interface
#define adarray_create(darr, type)\
    do {\
        _adarray_create((darr), ADARRAY_DEFAULT_CAPACITY, sizeof(type));\
    } while(0)

#define adarray_destroy(darr)\
    do {\
        _adarray_destroy((darr));\
    } while(0)

#define adarray_resize(darr)\
    do {\
        (void)_adarray_resize((darr));\
    } while(0)

#define adarray_push(darr, val)\
    do {\
        typeof(val) temp = val;\
        (void)_adarray_push((darr), (&temp));\
    } while(0)


#define adarray_insert(darr, idx, val)\
    do {\
        typeof(val) temp = val;\
        (void)_adarray_insert((darr), (idx), (&temp));\
    } while(0)


#define adarray_pop(darr, dest)\
    do {\
        (void)_adarray_pop((darr), (dest));\
    } while(0)

#define adarray_pop_at(darr, idx, dest)\
    do {\
        (void)_adarray_pop_at((darr), (idx), (dest));\
    } while(0)
