#include "adarray.h"
#include "core/memory.h"
#include "core/logger.h"

#include <stddef.h>

void _adarray_create(adarray* arr, u64 capacity, u64 stride) {
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

void _adarray_destroy(adarray* arr) {
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

adarray* _adarray_resize(adarray* arr) {
    if (!arr) {
        FATAL("array is null");
        return arr;
    }

    u64 old_size = arr->capacity * arr->stride;
    u64 new_capacity = arr->capacity == 0 ? 1 : arr->capacity * ADARRAY_RESIZE_FACTOR;
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

adarray* _adarray_push(adarray* arr, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }
    if(arr->length == arr->capacity) {
        FATAL("array is full, please use 'adarray_resize' to resize it");
        return arr;
    }
    u8* dest = (u8*)arr->data + arr->length * arr->stride;
    snmcopy(dest, v_ptr, arr->stride);
    arr->length++;

#if defined(SN_DEBUG)
    if(arr->length == arr->capacity) {
        DEBUG("array is full, please use 'adarray_resize' to resize it");
        return arr;
    }
#endif
    return arr;
}

adarray* _adarray_insert(adarray* arr, u64 idx, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }

    if(arr->length == arr->capacity) {
        FATAL("array is full, please use 'adarray_resize' to resize it");
        return arr;
    }

    if(idx > arr->length) {
        FATAL("index out of bounds");
        return arr;
    }

    u8* base = (u8*)arr->data;

    snmmove(base + (idx + 1) * arr->stride,
        base + idx * arr->stride,
        arr->stride * (arr->length - idx));
    snmcopy(base + idx * arr->stride, v_ptr, arr->stride);
    arr->length++;

#if defined(SN_DEBUG)
    if(arr->length == arr->capacity) {
        WARN("array is full, please use 'adarray_resize' to resize it");
        return arr;
    }
#endif
    return arr;
}

adarray* _adarray_pop(adarray* arr, void* dest) {
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
        return arr;
    }
#endif
    return arr;

}

adarray* _adarray_pop_at(adarray* arr, u64 idx, void* dest) {
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
        return arr;
    }
#endif
    return arr;
}
