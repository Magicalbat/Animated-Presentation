#ifndef BASE_MEM_H
#define BASE_MEM_H

#ifdef __cplusplus
extern "C" { 
#endif

#include <stdlib.h>
#include <string.h>

#include "base_defs.h"

// This is adapted from my own library mg_arena.h
// https://github.com/Magicalbat/mg_arena

typedef struct marena_malloc_node {
    struct marena_malloc_node* prev;
    u64 size;
    u64 pos;
    u8* data;
} marena_malloc_node; 

typedef struct {
    u64 pos;

    u64 size;
    u64 block_size;
    u32 align;

    union {
        struct {
            marena_malloc_node* cur_node;
        } malloc_backend;
        struct {
            u64 commit_pos;
        } reserve_backend;
    };
} marena;

typedef struct {
    u64 desired_max_size;
    u32 desired_block_size;
    u32 align;
} marena_desc;

typedef struct {
    marena* arena;
    u64 pos;
} marena_temp;

AP_EXPORT marena* marena_create(const marena_desc* desc);
AP_EXPORT void    marena_destroy(marena* arena);
AP_EXPORT void*   marena_push(marena* arena, u64 size);
AP_EXPORT void*   marena_push_zero(marena* arena, u64 size);
AP_EXPORT void*   marena_push_noalign(marena* arena, u64 size);
AP_EXPORT void*   marena_push_zero_noalign(marena* arena, u64 size);
AP_EXPORT void    marena_pop(marena* arena, u64 size);
AP_EXPORT void    marena_pop_to(marena* arena, u64 pos);
AP_EXPORT void    marena_reset(marena* arena);

AP_EXPORT marena_temp marena_temp_begin(marena* arena);
AP_EXPORT void        marena_temp_end(marena_temp temp);

AP_EXPORT void marena_scratch_set_desc(marena_desc* desc);
AP_EXPORT marena_temp marena_scratch_get(marena** conflicts, u32 num_conflicts);
AP_EXPORT void marena_scratch_release(marena_temp scratch);

#ifdef __cplusplus
}
#endif

#endif // BASE_MEM_H
