#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <stdlib.h>
#include <string.h>

#include "base_defs.h"

// TODO: include alignment?
// will this break some of the marena_pop calls?

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

marena* marena_create(const marena_desc* desc);
void    marena_destroy(marena* arena);
void*   marena_push(marena* arena, u64 size);
void*   marena_push_zero(marena* arena, u64 size);
void    marena_pop(marena* arena, u64 size);
void    marena_pop_to(marena* arena, u64 pos);
void    marena_reset(marena* arena);

marena_temp marena_temp_begin(marena* arena);
void        marena_temp_end(marena_temp temp);

void marena_scratch_set_desc(marena_desc* desc);
marena_temp marena_scratch_get(marena** conflicts, u32 num_conflicts);
void marena_scratch_release(marena_temp scratch);

#endif // BASE_MEM_H
