#include "darray.h"
#include "core/memory.h"
#include "core/logger.h"

#include <stddef.h>

void _darray_create(darray* arr, u64 capacity, u64 stride) {
    if(!arr) {
        FATAL("array is null");
        return;
    }
    if(capacity == 0) {
        FATAL("invalid capacity");
        return;
    }
    arr->capacity = capacity;
    arr->length = 0;
    arr->stride = stride;
    arr->data = snmalloc(capacity * stride, MEM_TAG_DARRAY);
    if(!arr->data) {
        FATAL("memory allocation failed");
        return;
    }
}

void _darray_destroy(darray* arr) {
    if(!arr) {
        FATAL("array is null");
        return;
    }
    snmfree(arr->data, arr->capacity * arr->stride, MEM_TAG_DARRAY);
    arr->data = NULL;
    arr->capacity = 0;
    arr->length = 0;
    arr->stride = 0;
}

darray* _darray_resize(darray* arr) {
    if (!arr) {
        FATAL("array is null");
        return arr;
    }

    u64 old_size = arr->capacity * arr->stride;
    u64 new_capacity = arr->capacity == 0 ? 1 : arr->capacity * 2;
    u64 new_size = new_capacity * arr->stride;

    void* new_mem = snmalloc(new_size, MEM_TAG_DARRAY);
    if (!new_mem) {
        FATAL("resize failed");
        return arr;
    }

    snmcopy(new_mem, arr->data, arr->capacity * arr->stride);

    snmfree(arr->data, old_size, MEM_TAG_DARRAY);

    arr->data = new_mem;
    arr->capacity = new_capacity;

    return arr;
}

darray* _darray_push(darray* arr, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }
    if(arr->length == arr->capacity) {
        arr = _darray_resize(arr);
        if(!arr) {
            FATAL("array is null");
            return NULL;
        }
    }
    u8* dest = (u8*)arr->data + arr->length * arr->stride;
    snmcopy(dest, v_ptr, arr->stride);
    arr->length++;

#if defined(SN_DEBUG)
    if(arr->length == arr->capacity) {
        DEBUG("array is full, please use 'darray_resize' to resize it");
    }
#endif
    return arr;
}

darray* _darray_insert(darray* arr, u64 idx, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }

    if(idx > arr->length) {
        FATAL("index out of bounds");
        return arr;
    }

    if(arr->length == arr->capacity) {
        arr = _darray_resize(arr);
        if(!arr) {
            FATAL("array is null");
            return NULL;
        }
    }

    u8* base = (u8*)arr->data;

    snmmove(base + (idx + 1) * arr->stride,
        base + idx * arr->stride,
        arr->stride * (arr->length - idx));
    snmcopy(base + idx * arr->stride, v_ptr, arr->stride);
    arr->length++;

#if defined(SN_DEBUG)
    if(arr->length == arr->capacity) {
        DEBUG("array is full, please use 'darray_resize' to resize it");
    }
#endif
    return arr;
}

darray* _darray_pop(darray* arr, void* dest) {
    if (!arr || arr->length == 0) {
        FATAL("invalid input");
        return arr;
    }
    arr->length--;
    u8* src = (u8*)arr->data + arr->length * arr->stride;

    if (dest) {
        snmcopy(dest, src, arr->stride);
    }

#if defined(SN_DEBUG)
    if(arr->length == 0) {
        DEBUG("array is empty");
    }
#endif
    return arr;

}

darray* _darray_pop_at(darray* arr, u64 idx, void* dest) {
    if(!arr || !arr->data) {
        FATAL("invalid input");
        return arr;
    }

    if(idx >= arr->length) {
        FATAL("index out of bounds");
        return arr;
    }

    u8* base = (u8*)arr->data;
    u8* target = base + idx * arr->stride;

    if(dest) {
        snmcopy(dest, target, arr->stride);
    }

    snmmove(target,
        target + arr->stride,
        (arr->length - idx - 1) * arr->stride);
    arr->length--;

#if defined(SN_DEBUG)
    if(arr->length == 0) {
        DEBUG("array is empty");
    }
#endif
    return arr;
}
