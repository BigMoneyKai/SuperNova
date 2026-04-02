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
linear_allocator engine_linear = {0};
general_allocator array_general = {0};
general_allocator darray_general = {0};
general_allocator ring_buffer_general = {0};
general_allocator bst_general = {0};
general_allocator texture_general = {0};
general_allocator material_instance_general = {0};
general_allocator renderer_general = {0};
general_allocator game_general = {0};
general_allocator transform_general = {0};
general_allocator entity_general = {0};
general_allocator entity_node_general = {0};
general_allocator scene_general = {0};

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

static void release_stack_usage(stack_allocator* stack, u64 mark) {
    if(!mss || !stack || !stack->start || mark >= stack->offset) {
        return;
    }

    u8* base = stack->start;
    u64 cursor = mark;

    while(cursor < stack->offset) {
        mem_header* header = (mem_header*)(base + cursor);
        u64 allocation_size = header->ud_size;
        memtag tag = header->tag;

        if(mss->total_size >= allocation_size) {
            mss->total_size -= allocation_size;
        } else {
            mss->total_size = 0;
        }

        if(tag < TAG_MAX_NUM) {
            if(mss->tagged_sizes[tag] >= allocation_size) {
                mss->tagged_sizes[tag] -= allocation_size;
            } else {
                mss->tagged_sizes[tag] = 0;
            }
        }

        cursor += sizeof(mem_header) + header->padding + allocation_size;
    }
}

static b8 init_allocators(void) {
    if(!stack_init(&job_stack, JOB_STACK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!stack_init(&string_stack, STRING_STACK_SIZE, ALIGNMENT)) return SN_FALSE;

    if(!linear_init(&engine_linear, ENGINE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!pool_init(&array_pool32, ARRAY_POOL_TOTAL_SIZE, ARRAY_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!pool_init(&darray_pool32, DARRAY_POOL_TOTAL_SIZE, DARRAY_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!pool_init(&ring_buffer_pool32, RING_POOL_TOTAL_SIZE, RING_POOL_BLOCK_SIZE, ALIGNMENT)) return SN_FALSE;

    if(!general_init(&array_general, ARRAY_POOL_TOTAL_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&darray_general, DARRAY_POOL_TOTAL_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&ring_buffer_general, RING_POOL_TOTAL_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&bst_general, BST_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&texture_general, TEXTURE_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&material_instance_general, MATERIAL_INSTANCE_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&renderer_general, RENDERER_HEAP_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&game_general, GAME_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&transform_general, TRANSFORM_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&entity_general, ENTITY_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&entity_node_general, ENTITY_NODE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;
    if(!general_init(&scene_general, SCENE_LINEAR_SIZE, ALIGNMENT)) return SN_FALSE;

    return SN_TRUE;
}

static void destroy_allocators(void) {
    stack_destroy(&job_stack);
    stack_destroy(&string_stack);

    pool_destroy(&array_pool32);
    pool_destroy(&darray_pool32);
    pool_destroy(&ring_buffer_pool32);

    linear_destroy(&engine_linear);
    general_destroy(&array_general);
    general_destroy(&darray_general);
    general_destroy(&ring_buffer_general);
    general_destroy(&bst_general);
    general_destroy(&texture_general);
    general_destroy(&material_instance_general);
    general_destroy(&renderer_general);
    general_destroy(&game_general);
    general_destroy(&transform_general);
    general_destroy(&entity_general);
    general_destroy(&entity_node_general);
    general_destroy(&scene_general);
}

static u64 accounted_size_for_tag(memtag tag, u64 requested_size) {
    switch(tag) {
        case MEM_TAG_ARRAY:
            return requested_size;
        case MEM_TAG_DARRAY:
            return requested_size;
        case MEM_TAG_RING_BUFFER:
            return requested_size;
        case MEM_TAG_TEXTURE:
            return requested_size;
        case MEM_TAG_MATERIAL_INSTANCE:
            return requested_size;
        case MEM_TAG_RENDERER:
            return requested_size;
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
            block = general_alloc(&array_general, size);
            break;
        case MEM_TAG_DARRAY:
            block = general_alloc(&darray_general, size);
            break;
        case MEM_TAG_RING_BUFFER:
            block = general_alloc(&ring_buffer_general, size);
            break;
        case MEM_TAG_BST:
            block = general_alloc(&bst_general, size);
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
            block = general_alloc(&texture_general, size);
            break;
        case MEM_TAG_MATERIAL_INSTANCE:
            block = general_alloc(&material_instance_general, size);
            break;
        case MEM_TAG_RENDERER:
            block = general_alloc(&renderer_general, size);
            break;
        case MEM_TAG_GAME:
            block = general_alloc(&game_general, size);
            break;
        case MEM_TAG_TRANSFORM:
            block = general_alloc(&transform_general, size);
            break;
        case MEM_TAG_ENTITY:
            block = general_alloc(&entity_general, size);
            break;
        case MEM_TAG_ENTITY_NODE:
            block = general_alloc(&entity_node_general, size);
            break;
        case MEM_TAG_SCENE:
            block = general_alloc(&scene_general, size);
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
            general_free(&array_general, block, size);
            break;
        case MEM_TAG_DARRAY:
            general_free(&darray_general, block, size);
            break;
        case MEM_TAG_RING_BUFFER:
            general_free(&ring_buffer_general, block, size);
            break;
        case MEM_TAG_TEXTURE:
            general_free(&texture_general, block, size);
            break;
        case MEM_TAG_MATERIAL_INSTANCE:
            general_free(&material_instance_general, block, size);
            break;
        case MEM_TAG_RENDERER:
            general_free(&renderer_general, block, size);
            break;
        case MEM_TAG_BST:
            general_free(&bst_general, block, size);
            break;
        case MEM_TAG_STRING:
        case MEM_TAG_JOB:
            WARN("tag '%s' does not support individual free", tag < TAG_MAX_NUM ? tag_strs[tag] : "invalid");
            return;
        case MEM_TAG_ENGINE:
            WARN("tag '%s' does not support individual free", tag < TAG_MAX_NUM ? tag_strs[tag] : "invalid");
            return;
        case MEM_TAG_GAME:
            general_free(&game_general, block, size);
            break;
        case MEM_TAG_TRANSFORM:
            general_free(&transform_general, block, size);
            break;
        case MEM_TAG_ENTITY:
            general_free(&entity_general, block, size);
            break;
        case MEM_TAG_ENTITY_NODE:
            general_free(&entity_node_general, block, size);
            break;
        case MEM_TAG_SCENE:
            general_free(&scene_general, block, size);
            break;
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

void snmmove(void* dest, const void* src, u64 size) {
    if(!dest || !src) {
        WARN("snmmove received null pointer");
        return;
    }

    platform_move_memory(dest, src, size);
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

u64 snm_string_mark(void) {
    return stack_mark(&string_stack);
}

void snm_string_reset_to_mark(u64 mark) {
    release_stack_usage(&string_stack, mark);
    stack_reset_to_mark(&string_stack, mark);
}

void snm_string_reset(void) {
    release_stack_usage(&string_stack, 0);
    stack_reset(&string_stack);
}

u64 snm_job_mark(void) {
    return stack_mark(&job_stack);
}

void snm_job_reset_to_mark(u64 mark) {
    release_stack_usage(&job_stack, mark);
    stack_reset_to_mark(&job_stack, mark);
}

void snm_job_reset(void) {
    release_stack_usage(&job_stack, 0);
    stack_reset(&job_stack);
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
