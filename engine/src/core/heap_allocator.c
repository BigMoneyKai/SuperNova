#include "allocator.h"
#include "logger.h"
#include "platform/platform.h"

#include <stddef.h>

static b8 is_valid(u64 size) {
    return size != 0 && (size & (size - 1)) == 0;
}

b8 heap_init(heap_allocator* heap, u64 total_size, u64 alignment) {
    if(!heap) {
        FATAL("heap allocator is null");
        return SN_FALSE;
    }
    if(!is_valid(alignment)) {
        FATAL("Invalid alignment");
        return SN_FALSE;
    }
    heap->total_size = total_size;
    heap->alignment = alignment;
    heap->start = platform_allocate_aligned(total_size, alignment);
    heap->is_allocated = SN_FALSE;
    return heap->start != NULL ? SN_TRUE : SN_FALSE;
}

void* heap_alloc(heap_allocator* heap) {
    if(!heap || !heap->start) {
        FATAL("heap allocator is not initialized");
        return NULL;
    }
    if(heap->is_allocated) {
        ERROR("heap allocator supports only one active allocation");
        return NULL;
    }
    heap->is_allocated = SN_TRUE;
    return heap->start;
}

void heap_free(heap_allocator* heap) {
    if(!heap || !heap->start) {
        return;
    }
    heap->is_allocated = SN_FALSE;
}

void heap_destroy(heap_allocator* heap) {
    if(!heap || !heap->start) {
        return;
    }
    platform_free(heap->start, SN_TRUE);
    heap->start = NULL;
    heap->total_size = 0;
    heap->alignment = 0;
    heap->is_allocated = SN_FALSE;
}
