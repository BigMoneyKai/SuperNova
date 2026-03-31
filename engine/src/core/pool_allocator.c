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

b8 pool_init(pool_allocator* pool, u64 total_size, u64 block_size, u64 alignment) {
    if(!is_valid(alignment)) {
        FATAL("Invalid alignment");
        return SN_FALSE;
    }

    if(total_size < block_size) {
        FATAL("total size should be larger than block size");
        return SN_FALSE;
    }

    block_size = (block_size < sizeof(free_block)) ? sizeof(free_block) : block_size;
    block_size = align_up(block_size, alignment);

    pool->total_size = total_size;
    pool->block_size = block_size;
    pool->alignment = alignment;
    pool->raw = (addr)platform_allocate(pool->total_size + alignment);
    pool->start = (u8*)align_up(pool->raw, alignment);

    u8* base = pool->start;
    pool->free_list = (free_block*)base;

    u64 count = total_size / block_size;
    pool->block_count = count;
    for(u64 i = 0; i < count - 1; i++) {
        free_block* curr = (free_block*)(base + i * block_size);
        curr->next = (free_block*)(base + (i + 1) * block_size);
        curr->tag = MEM_TAG_UNKNOWN;
    }
    ((free_block*)(base + (count - 1) * block_size))->next = NULL;
    ((free_block*)(base + (count - 1) * block_size))->tag = MEM_TAG_UNKNOWN;

    return SN_TRUE;
}

void* pool_alloc(pool_allocator* pool, memtag tag) {
    if(!pool || !pool->start) {
        FATAL("pool allocator is not initialized");
        return NULL;
    }
    if(!pool->free_list) {
        FATAL("free list is empty");
        return NULL;
    }
    free_block* block = pool->free_list;
    block->tag = tag;
    pool->free_list = block->next;
    return (void*)block;
}

void pool_free(pool_allocator* pool, void* block) {
    if(!block) {
        FATAL("block is empty");
        return;
    }
    u8* start = pool->start;
    u8* end = start + pool->total_size;

    if ((u8*)block < start || (u8*)block >= end) {
        FATAL("invalid pointer");
        return;
    }

    u64 offset = (u64)((u8*)block - start);
    if (offset % pool->block_size != 0) {
        FATAL("misaligned block");
        return;
    }

    free_block* fb = block;
    fb->next = pool->free_list;
    fb->tag = MEM_TAG_UNKNOWN;
    pool->free_list = fb;
}

void pool_destroy(pool_allocator* pool) {
    if(!pool || !pool->raw) {
        return;
    }
    platform_free((void*)pool->raw, SN_FALSE);
    pool->raw = 0;
    pool->start = NULL;
    pool->free_list = NULL;
    pool->block_size = 0;
    pool->block_count = 0;
    pool->total_size = 0;
    pool->alignment = 0;
}
