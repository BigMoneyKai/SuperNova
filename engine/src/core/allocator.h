#pragma once

#include "memory.h"
#include "defines.h"

//    ┌───────────────┬───────────────┬──────────────────────────────┐
//    │   header      │    padding    │         user data            │
//    │               │               │                              │
//    └───────────────┴───────────────┴──────────────────────────────┘

typedef struct mem_header {
    u64 ud_size;
    u64 padding;
    memtag tag;
#if SN_DEBUG
    const char* file;
    i32 line;
#endif
} mem_header;

// ===== Linear Allocator =====
typedef struct linear_allocator {
    addr raw;
    u8* start;
    u64 total_size;
    u64 offset;
    u64 alignment;

} linear_allocator;

b8 linear_init(linear_allocator* linear, u64 total_size, u64 alignment);
void* linear_alloc(linear_allocator* linear, u64 size, memtag tag);
void linear_reset(linear_allocator* linear);
void linear_destroy(linear_allocator* linear);

// ===== Pool Allocator =====
typedef struct free_block {
    memtag tag;
    struct free_block* next;
#if SN_DEBUG
    const char* file;
    i32 line;
#endif
} free_block;

typedef struct pool_allocator {
    addr raw;
    u8* start;
    u64 total_size;
    u64 block_size;
    u64 block_count;
    u64 alignment;

    free_block* free_list;

} pool_allocator;

b8 pool_init(pool_allocator* pool, u64 total_size, u64 block_size, u64 alignment);
void* pool_alloc(pool_allocator* pool, memtag tag);
void pool_free(pool_allocator* pool, void* block);
void pool_destroy(pool_allocator* pool);

// ===== Stack Allocator =====
typedef struct stack_allocator {
    addr raw;
    u8* start;
    u64 total_size;
    u64 offset;
    u64 alignment;

} stack_allocator;

b8 stack_init(stack_allocator* stack, u64 total_size, u64 alignment);
void* stack_alloc(stack_allocator* stack, u64 size, memtag tag);
u64 stack_mark(stack_allocator* stack);
void stack_reset_to_mark(stack_allocator* stack, u64 mark);
void stack_reset(stack_allocator* stack);
void stack_destroy(stack_allocator* stack);

// ===== Heap Allocator =====
typedef struct heap_allocator {
    u8* start;
    u64 total_size;
    u64 alignment;
    b8 is_allocated;
} heap_allocator;

b8 heap_init(heap_allocator* heap, u64 total_size, u64 alignment);
void* heap_alloc(heap_allocator* heap);
void heap_free(heap_allocator* heap);
void heap_destroy(heap_allocator* heap);
