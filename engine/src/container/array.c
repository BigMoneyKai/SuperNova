#include "array.h"
#include "core/memory.h"
#include "core/logger.h"

#include <stddef.h>

void _array_create(array* arr, u64 capacity, u64 stride) {
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
    arr->data = snmalloc(capacity * stride, MEM_TAG_ARRAY);
    if(!arr->data) {
        FATAL("memory allocation failed");
        return;
    }
}

void _array_destroy(array* arr) {
    if(!arr) {
        FATAL("array is null");
        return;
    }
    snmfree(arr->data, arr->capacity * arr->stride, MEM_TAG_ARRAY);
    arr->data = NULL;
    arr->capacity = 0;
    arr->length = 0;
    arr->stride = 0;
}

array* _array_push(array* arr, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }
    if(arr->length == arr->capacity) {
        FATAL("array is full");
        return arr;
    }
    u8* dest = (u8*)arr->data + arr->length * arr->stride;
    snmcopy(dest, v_ptr, arr->stride);
    arr->length++;

#if defined(SN_DEBUG)
    if(arr->length == arr->capacity) {
        DEBUG("array is full");
    }
#endif
    return arr;
}

array* _array_insert(array* arr, u64 idx, const void* v_ptr) {
    if(!arr || !v_ptr) {
        FATAL("invalid input");
        return arr;
    }

    if(idx > arr->length) {
        FATAL("index out of bounds");
        return arr;
    }

    if(arr->length == arr->capacity) {
        FATAL("array is full");
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
        DEBUG("array is full");
    }
#endif
    return arr;
}

array* _array_pop(array* arr, void* dest) {
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

array* _array_pop_at(array* arr, u64 idx, void* dest) {
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
