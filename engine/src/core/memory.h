#pragma once

#include "defines.h"

typedef struct stack_allocator stack_allocator;
typedef struct pool_allocator pool_allocator;
typedef struct linear_allocator linear_allocator;
typedef struct heap_allocator heap_allocator;

#define B(x) ((u64)(x))
#define KB(x) (B(x) * 1024ULL)
#define MB(x) (KB(x) * 1024ULL)

#define JOB_STACK_SIZE                MB(1)
#define STRING_STACK_SIZE             MB(1)

#define ARRAY_POOL_BLOCK_SIZE         B(32)
#define ARRAY_POOL_TOTAL_SIZE         KB(32)

#define DARRAY_POOL_BLOCK_SIZE        B(32)
#define DARRAY_POOL_TOTAL_SIZE        KB(32)

#define RING_POOL_BLOCK_SIZE          B(32)
#define RING_POOL_TOTAL_SIZE          KB(32)

#define BST_LINEAR_SIZE               MB(1)
#define ENGINE_LINEAR_SIZE            MB(2)

#define TEXTURE_HEAP_SIZE             MB(8)
#define MATERIAL_INSTANCE_HEAP_SIZE   MB(2)
#define RENDERER_HEAP_SIZE            MB(4)

#define GAME_LINEAR_SIZE              MB(2)
#define TRANSFORM_LINEAR_SIZE         MB(1)
#define ENTITY_LINEAR_SIZE            MB(1)
#define ENTITY_NODE_LINEAR_SIZE       MB(1)
#define SCENE_LINEAR_SIZE             MB(1)

typedef enum memtag {
    MEM_TAG_UNKNOWN,
    MEM_TAG_ARRAY,
    MEM_TAG_DARRAY,
    MEM_TAG_RING_BUFFER,
    MEM_TAG_BST,
    MEM_TAG_STRING,
    MEM_TAG_ENGINE,
    MEM_TAG_JOB,
    MEM_TAG_TEXTURE,
    MEM_TAG_MATERIAL_INSTANCE,
    MEM_TAG_RENDERER,
    MEM_TAG_GAME,
    MEM_TAG_TRANSFORM,
    MEM_TAG_ENTITY,
    MEM_TAG_ENTITY_NODE,
    MEM_TAG_SCENE,

    TAG_MAX_NUM,
} memtag;

typedef struct memsys_state {
    u64 total_size;
    u64 tagged_sizes[TAG_MAX_NUM];
} memsys_state;

extern memsys_state* mss;

extern stack_allocator job_stack;
extern stack_allocator string_stack;
extern pool_allocator array_pool32;
extern pool_allocator darray_pool32;
extern pool_allocator ring_buffer_pool32;
extern linear_allocator bst_linear;
extern linear_allocator engine_linear;
extern heap_allocator texture_heap;
extern heap_allocator material_instance_heap;
extern heap_allocator renderer_heap;
extern linear_allocator game_linear;
extern linear_allocator transform_linear;
extern linear_allocator entity_linear;
extern linear_allocator entity_node_linear;
extern linear_allocator scene_linear;

SNAPI b8 snminit(u64 size, void* state);
SNAPI b8 snmquit(void* state);
SNAPI void* snmalloc(u64 size, memtag tag);
SNAPI void snmfree(void* block, u64 size, memtag tag);
SNAPI void snmcopy(void* dest, const void* src, u64 size);
SNAPI void snmset(void* block, i32 value, u64 size);
SNAPI void snmzero(void* block, u64 size);

SNAPI void get_meminfo(memtag tag, char* buffer, u64 bufsize);
SNAPI void print_mstats(void);
