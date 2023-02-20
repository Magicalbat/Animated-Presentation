#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <stdlib.h>
#include <string.h>

#include "base_defs.h"

// This is adapted from my own library mg_arena.h
// https://github.com/Magicalbat/mg_arena

typedef struct arena_malloc_node {
    struct arena_malloc_node* prev;
    u64 size;
    u64 pos;
    u8* data;
} arena_malloc_node; 

typedef struct {
    u64 pos;

    u64 size;
    u64 block_size;

    union {
        struct {
            arena_malloc_node* cur_node;
        } malloc_backend;
        struct {
            u64 commit_pos;
        } reserve_backend;
    };
} arena;

typedef struct {
    u64 desired_max_size;
    u32 desired_block_size;
    u32 align;
} arena_desc;

typedef struct {
    arena* arena;
    u64 pos;
} arena_temp;

arena* arena_create(const arena_desc* desc);
void   arena_destroy(arena* arena);
void*  arena_push(arena* arena, u64 size);
void*  arena_push_zero(arena* arena, u64 size);
void   arena_pop(arena* arena, u64 size);
void   arena_pop_to(arena* arena, u64 pos);
void   arena_reset(arena* arena);

arena_temp arena_temp_begin(arena* arena);
void       arena_temp_end(arena_temp temp);

void arena_scratch_set_desc(arena_desc* desc);
arena_temp arena_scratch_get(arena** conflicts, u32 num_conflicts);
void arena_scratch_release(arena_temp scratch);

#endif // BASE_MEM_H
