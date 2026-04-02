#include "allocator.h"
#include "logger.h"
#include "platform/platform.h"

#include <stddef.h>

static b8 is_valid(u64 size) {
    return size != 0 && (size & (size - 1)) == 0;
}

b8 general_init(general_allocator* alloc, u64 total_size, u64 alignment) {
    if(!alloc) {
        FATAL("general allocator is null");
        return SN_FALSE;
    }
    if(!is_valid(alignment)) {
        FATAL("Invalid alignment");
        return SN_FALSE;
    }
    alloc->total_size = total_size;
    alloc->used = 0;
    alloc->alignment = alignment;
    return SN_TRUE;
}

void* general_alloc(general_allocator* alloc, u64 size) {
    if(!alloc) {
        FATAL("general allocator is null");
        return NULL;
    }
    if(size == 0) {
        WARN("general allocator received zero-size allocation");
        return NULL;
    }
    if(alloc->total_size && alloc->used + size > alloc->total_size) {
        ERROR("general allocator budget exceeded");
        return NULL;
    }

    void* block = platform_allocate_aligned(size, alloc->alignment);
    if(!block) {
        ERROR("general allocation failed");
        return NULL;
    }

    alloc->used += size;
    return block;
}

void general_free(general_allocator* alloc, void* block, u64 size) {
    if(!alloc || !block) {
        return;
    }

    platform_free(block, SN_TRUE);

    if(alloc->used >= size) {
        alloc->used -= size;
    } else {
        alloc->used = 0;
    }
}

void general_destroy(general_allocator* alloc) {
    if(!alloc) {
        return;
    }
    alloc->total_size = 0;
    alloc->used = 0;
    alloc->alignment = 0;
}
