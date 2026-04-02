#include <core/logger.h>
#include <core/asserts.h>
#include <core/memory.h>
#include <platform/platform.h>
#include <container/array.h>
#include <container/darray.h>
#include <container/adarray.h>
#include <defines.h>

#include <stdio.h>
#include <stdlib.h>

static b8 test_general_allocators(void) {
    void* array_block = snmalloc(sizeof(i32) * 16, MEM_TAG_ARRAY);
    void* darray_block = snmalloc(sizeof(i32) * 16, MEM_TAG_DARRAY);
    void* ring_block = snmalloc(256, MEM_TAG_RING_BUFFER);
    void* bst_block = snmalloc(96, MEM_TAG_BST);
    void* texture_a = snmalloc(256, MEM_TAG_TEXTURE);
    void* texture_b = snmalloc(512, MEM_TAG_TEXTURE);
    void* entity_block = snmalloc(128, MEM_TAG_ENTITY);
    void* scene_block = snmalloc(96, MEM_TAG_SCENE);

    if(!array_block || !darray_block || !ring_block || !bst_block || !texture_a || !texture_b || !entity_block || !scene_block) {
        return SN_FALSE;
    }

    snmfree(scene_block, 96, MEM_TAG_SCENE);
    snmfree(entity_block, 128, MEM_TAG_ENTITY);
    snmfree(texture_b, 512, MEM_TAG_TEXTURE);
    snmfree(texture_a, 256, MEM_TAG_TEXTURE);
    snmfree(bst_block, 96, MEM_TAG_BST);
    snmfree(ring_block, 256, MEM_TAG_RING_BUFFER);
    snmfree(darray_block, sizeof(i32) * 16, MEM_TAG_DARRAY);
    snmfree(array_block, sizeof(i32) * 16, MEM_TAG_ARRAY);
    return SN_TRUE;
}

static b8 test_stack_allocators(void) {
    u64 string_mark = snm_string_mark();
    char* temp_string = snmalloc(64, MEM_TAG_STRING);
    if(!temp_string) {
        return SN_FALSE;
    }
    snmcopy(temp_string, "supernova", 10);
    snm_string_reset_to_mark(string_mark);

    u64 job_mark = snm_job_mark();
    void* job_mem = snmalloc(128, MEM_TAG_JOB);
    if(!job_mem) {
        return SN_FALSE;
    }
    snm_job_reset_to_mark(job_mark);
    return SN_TRUE;
}

static b8 test_containers(void) {
    array fixed = {0};
    darray dynamic = {0};
    adarray manual = {0};
    i32 out = 0;

    array_create(&fixed, i32);
    darray_create(&dynamic, i32);
    adarray_create(&manual, i32);

    for(i32 i = 0; i < 16; ++i) {
        array_push(&fixed, i);
        darray_push(&dynamic, i);
        adarray_push(&manual, i);
    }

    darray_push(&dynamic, 16);
    adarray_resize(&manual);
    adarray_push(&manual, 16);

    array_pop(&fixed, &out);
    if(out != 15) return SN_FALSE;
    darray_pop(&dynamic, &out);
    if(out != 16) return SN_FALSE;
    adarray_pop(&manual, &out);
    if(out != 16) return SN_FALSE;

    array_destroy(&fixed);
    darray_destroy(&dynamic);
    adarray_destroy(&manual);
    return SN_TRUE;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    memsys_state app_state;

    if(!init_logging()) {
        return 1;
    }

    if(!snminit(0, &app_state)) {
        quit_logging();
        return 1;
    }

    if(!test_general_allocators() || !test_stack_allocators() || !test_containers()) {
        snmquit(&app_state);
        quit_logging();
        return 1;
    }


    platform_state state = {0};
    if(platform_startup(&state, "Supernova Engine", 100, 100, 1280, 720)) {
        platform_shutdown(&state);
    } else {
        WARN("platform startup failed; memory smoke test still passed");
    }

    snmquit(&app_state);
    quit_logging();
    return 0;
}
