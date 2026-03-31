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

b8 stack_init(stack_allocator* stack, u64 total_size, u64 alignment)  {
    if(!is_valid(alignment)) {
        FATAL("Invalid alignment");
        return SN_FALSE;
    }

    stack->alignment = alignment;
    stack->raw = (addr)platform_allocate(total_size + alignment);
    stack->start = (u8*)align_up(stack->raw, alignment);
    stack->total_size = total_size;
    stack->offset = 0;
    return SN_TRUE;
}

void* stack_alloc(stack_allocator* stack, u64 size, memtag tag) {
    if(!stack || !stack->start) {
        ERROR("stack allocator is not initialized");
        return NULL;
    }
    if(sizeof(mem_header) + size + stack->alignment + stack->offset > stack->total_size) {
        ERROR("memory pool is full");
        return NULL;
    }

    u8* base = (u8*)stack->start;
    void* aligned = (void*)align_up((addr)(base + stack->offset + sizeof(mem_header)), stack->alignment);

    mem_header* header = (mem_header*)(base + stack->offset);
    header->padding = (u64)((u8*)aligned - (base + stack->offset + sizeof(mem_header)));
    header->ud_size = size;
    header->tag = tag;

    stack->offset += sizeof(mem_header) + header->padding + header->ud_size;
    return aligned;

}

u64 stack_mark(stack_allocator* stack) {
    return stack->offset;
}

void stack_reset_to_mark(stack_allocator* stack, u64 mark) {
    stack->offset = mark;
}

void stack_reset(stack_allocator* stack) {
    stack->offset = 0;
}

void stack_destroy(stack_allocator* stack) {
    if(!stack || !stack->raw) {
        return;
    }
    platform_free((void*)stack->raw, SN_FALSE);
    stack->start = NULL;
    stack->raw = 0;
    stack->total_size = 0;
    stack->offset = 0;
    stack->alignment = 0;
}
