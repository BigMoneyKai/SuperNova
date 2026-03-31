#include "allocator.h"
#include "logger.h"
#include "platform/platform.h"

#include <stddef.h>

static b8 is_valid(u64 size) {
    return size != 0 && (size & (size - 1)) == 0;
}

static u64 align_up(u64 raw, u64 alignment) {
    return (raw + alignment - 1) & ~(alignment - 1);
}

b8 linear_init(linear_allocator* linear, u64 total_size, u64 alignment)  {
    if(!is_valid(alignment)) {
        FATAL("Invalid alignment");
        return SN_FALSE;
    }

    linear->alignment = alignment;
    linear->raw = (addr)platform_allocate(total_size + alignment);
    linear->start = (u8*)align_up(linear->raw, alignment);
    linear->total_size = total_size;
    linear->offset = 0;
    return SN_TRUE;
}

void* linear_alloc(linear_allocator* linear, u64 size, memtag tag) {
    if(!linear || !linear->start) {
        ERROR("linear allocator is not initialized");
        return NULL;
    }
    if(sizeof(mem_header) + size + linear->alignment + linear->offset > linear->total_size) {
        ERROR("memory pool is full");
        return NULL;
    }

    u8* base = (u8*)linear->start;
    void* aligned = (void*)align_up((addr)(base + linear->offset + sizeof(mem_header)), linear->alignment);

    mem_header* header = (mem_header*)(base + linear->offset);
    header->padding = (u64)((u8*)aligned - (base + linear->offset + sizeof(mem_header)));
    header->ud_size = size;
    header->tag = tag;

    linear->offset += sizeof(mem_header) + header->padding + header->ud_size;
    return aligned;

}

void linear_reset(linear_allocator* linear) {
    linear->offset = 0;
}

void linear_destroy(linear_allocator* linear) {
    if(!linear || !linear->raw) {
        return;
    }
    platform_free((void*)linear->raw, SN_FALSE);
    linear->start = NULL;
    linear->raw = 0;
    linear->total_size = 0;
    linear->offset = 0;
    linear->alignment = 0;
}
