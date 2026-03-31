#include "memory.h"
#include "allocator.h"
#include "logger.h"
#include "platform/platform.h"

#include <stdio.h>

memsys_state* mss = 0;

stack_allocator job_stack = {0};
stack_allocator string_stack = {0};
pool_allocator array_pool32 = {0};
pool_allocator darray_pool32 = {0};
pool_allocator ring_buffer_pool32 = {0};
linear_allocator bst_linear = {0};
linear_allocator engine_linear = {0};
heap_allocator texture_heap = {0};
heap_allocator material_instance_heap = {0};
heap_allocator renderer_heap = {0};
linear_allocator game_linear = {0};
linear_allocator transform_linear = {0};
linear_allocator entity_linear = {0};
linear_allocator entity_node_linear = {0};
linear_allocator scene_linear = {0};

static const char* tag_strs[TAG_MAX_NUM] = {
    "unknown",
    "array",
    "dynamic array",
    "ring buffer",
    "bst",
    "string",
    "engine",
    "job",
    "texture",
    "material instance",
    "renderer",
    "game",
    "transform",
    "entity",
    "entity node",
    "scene",
};

static void fmt_size(u64 size, char* buf, u64 bufsize) {
    if(!buf || bufsize == 0) {
        return;
    }

    if(size < 1024ULL) {
        snprintf(buf, bufsize, "%.2f B", (f32)size);
    } else if(size < 1024ULL * 1024ULL) {
        snprintf(buf, bufsize, "%.2f KB", size / 1024.0f);
    } else if(size < 1024ULL * 1024ULL * 1024ULL) {
        snprintf(buf, bufsize, "%.2f MB", size / (1024.0f * 1024.0f));
    } else if(size < 1024ULL * 1024ULL * 1024ULL * 1024ULL) {
        snprintf(buf, bufsize, "%.2f GB", size / (1024.0f * 1024.0f * 1024.0f));
    } else {
        snprintf(buf, bufsize, "memory allocation too large");
    }
}

static b8 init_allocators(void) {
    if(!stack_init(&job_stack, JOB_STACK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!stack_init(&string_stack, STRING_STACK_SIZE, ALIGNMENT)) return SN_FALSE;

    if(!pool_init(&array_pool32, ARRAY_POOL_TOTAL_SIZE, ARRAY_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!pool_init(&darray_pool32, DARRAY_POOL_TOTAL_SIZE, DARRAY_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!pool_init(&ring_buffer_pool32, RING_POOL_TOTAL_SIZE, RING_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;

    if(!linear_init(&bst_linear, BST_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&engine_linear, ENGINE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&game_linear, GAME_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&transform_linear, TRANSFORM_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&entity_linear, ENTITY_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&entity_node_linear, ENTITY_NODE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!linear_init(&scene_linear, SCENE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;

    if(!heap_init(&texture_heap, TEXTURE_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!heap_init(&material_instance_heap, MATERIAL_INSTANCE_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!heap_init(&renderer_heap, RENDERER_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;

    return SN_TRUE;
}

static void destroy_allocators(void) {
    stack_destroy(&job_stack);
    stack_destroy(&string_stack);

    pool_destroy(&array_pool32);
    pool_destroy(&darray_pool32);
    pool_destroy(&ring_buffer_pool32);

    linear_destroy(&bst_linear);
    linear_destroy(&engine_linear);
    linear_destroy(&game_linear);
    linear_destroy(&transform_linear);
    linear_destroy(&entity_linear);
    linear_destroy(&entity_node_linear);
    linear_destroy(&scene_linear);

    heap_destroy(&texture_heap);
    heap_destroy(&material_instance_heap);
    heap_destroy(&renderer_heap);
}

static u64 accounted_size_for_tag(memtag tag, u64 requested_size) {
    switch(tag) {
        case MEM_TAG_ARRAY:
            return array_pool32.block_size;
        case MEM_TAG_DARRAY:
            return darray_pool32.block_size;
        case MEM_TAG_RING_BUFFER:
            return ring_buffer_pool32.block_size;
        case MEM_TAG_TEXTURE:
            return texture_heap.total_size;
        case MEM_TAG_MATERIAL_INSTANCE:
            return material_instance_heap.total_size;
        case MEM_TAG_RENDERER:
            return renderer_heap.total_size;
        default:
            return requested_size;
    }
}

b8 snminit(u64 size, void* state) {
    (void)size;

    if(!state) {
        FATAL("memory system state is null");
        return SN_FALSE;
    }

    mss = (memsys_state*)state;
    mss->total_size = 0;
    platform_zero_memory(mss->tagged_sizes, sizeof(mss->tagged_sizes));

    if(!init_allocators()) {
        FATAL("failed to initialize memory allocators");
        destroy_allocators();
        mss = 0;
        return SN_FALSE;
    }

    return SN_TRUE;
}

b8 snmquit(void* state) {
    if(!state) {
        FATAL("memory system state is null");
        return SN_FALSE;
    }

    mss = (memsys_state*)state;
    destroy_allocators();
    mss->total_size = 0;
    platform_zero_memory(mss->tagged_sizes, sizeof(mss->tagged_sizes));
    mss = 0;
    return SN_TRUE;
}

void* snmalloc(u64 size, memtag tag) {
    void* block = 0;
    u64 accounted_size = 0;

    if(!mss) {
        FATAL("memory system is not initialized");
        return 0;
    }

    if(size == 0) {
        WARN("requested allocation size is 0");
        return 0;
    }

    if(tag == MEM_TAG_UNKNOWN) {
        WARN("memory allocated with MEM_TAG_UNKNOWN");
    }

    switch(tag) {
        case MEM_TAG_ARRAY:
            if(size > array_pool32.block_size) {
                ERROR("requested size %llu exceeds array pool block size %llu", size, array_pool32.block_size);
                return 0;
            }
            block = pool_alloc(&array_pool32, tag);
            break;
        case MEM_TAG_DARRAY:
            if(size > darray_pool32.block_size) {
                ERROR("requested size %llu exceeds darray pool block size %llu", size, darray_pool32.block_size);
                return 0;
            }
            block = pool_alloc(&darray_pool32, tag);
            break;
        case MEM_TAG_RING_BUFFER:
            if(size > ring_buffer_pool32.block_size) {
                ERROR("requested size %llu exceeds ring buffer pool block size %llu", size, ring_buffer_pool32.block_size);
                return 0;
            }
            block = pool_alloc(&ring_buffer_pool32, tag);
            break;
        case MEM_TAG_BST:
            block = linear_alloc(&bst_linear, size, tag);
            break;
        case MEM_TAG_STRING:
            block = stack_alloc(&string_stack, size, tag);
            break;
        case MEM_TAG_ENGINE:
            block = linear_alloc(&engine_linear, size, tag);
            break;
        case MEM_TAG_JOB:
            block = stack_alloc(&job_stack, size, tag);
            break;
        case MEM_TAG_TEXTURE:
            block = heap_alloc(&texture_heap);
            break;
        case MEM_TAG_MATERIAL_INSTANCE:
            block = heap_alloc(&material_instance_heap);
            break;
        case MEM_TAG_RENDERER:
            block = heap_alloc(&renderer_heap);
            break;
        case MEM_TAG_GAME:
            block = linear_alloc(&game_linear, size, tag);
            break;
        case MEM_TAG_TRANSFORM:
            block = linear_alloc(&transform_linear, size, tag);
            break;
        case MEM_TAG_ENTITY:
            block = linear_alloc(&entity_linear, size, tag);
            break;
        case MEM_TAG_ENTITY_NODE:
            block = linear_alloc(&entity_node_linear, size, tag);
            break;
        case MEM_TAG_SCENE:
            block = linear_alloc(&scene_linear, size, tag);
            break;
        case MEM_TAG_UNKNOWN:
        default:
            block = linear_alloc(&engine_linear, size, MEM_TAG_UNKNOWN);
            break;
    }

    if(!block) {
        FATAL("memory allocation failure, tag=%u, size=%llu", (u32)tag, size);
        return 0;
    }

    accounted_size = accounted_size_for_tag(tag, size);
    mss->total_size += accounted_size;
    mss->tagged_sizes[tag] += accounted_size;
    return block;
}

void snmfree(void* block, u64 size, memtag tag) {
    u64 accounted_size = 0;

    if(!mss) {
        FATAL("memory system is not initialized");
        return;
    }

    if(!block) {
        WARN("attempted to free null block");
        return;
    }

    accounted_size = accounted_size_for_tag(tag, size);

    switch(tag) {
        case MEM_TAG_ARRAY:
            pool_free(&array_pool32, block);
            break;
        case MEM_TAG_DARRAY:
            pool_free(&darray_pool32, block);
            break;
        case MEM_TAG_RING_BUFFER:
            pool_free(&ring_buffer_pool32, block);
            break;
        case MEM_TAG_TEXTURE:
            if(block != texture_heap.start) {
                ERROR("invalid texture heap pointer");
                return;
            }
            heap_free(&texture_heap);
            break;
        case MEM_TAG_MATERIAL_INSTANCE:
            if(block != material_instance_heap.start) {
                ERROR("invalid material instance heap pointer");
                return;
            }
            heap_free(&material_instance_heap);
            break;
        case MEM_TAG_RENDERER:
            if(block != renderer_heap.start) {
                ERROR("invalid renderer heap pointer");
                return;
            }
            heap_free(&renderer_heap);
            break;
        case MEM_TAG_BST:
        case MEM_TAG_STRING:
        case MEM_TAG_ENGINE:
        case MEM_TAG_JOB:
        case MEM_TAG_GAME:
        case MEM_TAG_TRANSFORM:
        case MEM_TAG_ENTITY:
        case MEM_TAG_ENTITY_NODE:
        case MEM_TAG_SCENE:
        case MEM_TAG_UNKNOWN:
        default:
            WARN("tag '%s' does not support individual free", tag < TAG_MAX_NUM ? tag_strs[tag] : "invalid");
            return;
    }

    if(mss->total_size >= accounted_size) {
        mss->total_size -= accounted_size;
    } else {
        mss->total_size = 0;
    }

    if(tag < TAG_MAX_NUM) {
        if(mss->tagged_sizes[tag] >= accounted_size) {
            mss->tagged_sizes[tag] -= accounted_size;
        } else {
            mss->tagged_sizes[tag] = 0;
        }
    }
}

void snmcopy(void* dest, const void* src, u64 size) {
    if(!dest || !src) {
        WARN("snmcopy received null pointer");
        return;
    }

    platform_copy_memory(dest, src, size);
}

void snmset(void* block, i32 value, u64 size) {
    if(!block) {
        WARN("snmset received null block");
        return;
    }

    platform_set_memory(block, value, size);
}

void snmzero(void* block, u64 size) {
    if(!block) {
        WARN("snmzero received null block");
        return;
    }

    platform_zero_memory(block, size);
}

void get_meminfo(memtag tag, char* buffer, u64 bufsize) {
    if(!mss) {
        FATAL("memory system is not initialized");
        return;
    }

    if(!buffer || bufsize == 0) {
        return;
    }

    if(tag >= TAG_MAX_NUM) {
        snprintf(buffer, bufsize, "invalid tag");
        return;
    }

    fmt_size(mss->tagged_sizes[tag], buffer, bufsize);
}

void print_mstats(void) {
    char buf[256];

    if(!mss) {
        FATAL("memory system is not initialized");
        return;
    }

    fmt_size(mss->total_size, buf, sizeof(buf));
    INFO("[ total memory usage: %s ]", buf);

    for(u64 tag = 0; tag < TAG_MAX_NUM; ++tag) {
        fmt_size(mss->tagged_sizes[tag], buf, sizeof(buf));
        INFO("%s: %s", tag_strs[tag], buf);
    }
}
