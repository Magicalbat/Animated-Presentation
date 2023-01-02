#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <stdlib.h>
#include <string.h>

#include "base_defs.h"

// TODO: scratch arenas

typedef struct {
    u64 size;
    u64 cur;
    u64 cur_commit;
    // Data is stored in the memory following the struct
} arena_t;

typedef struct {
    u64 start_pos;
    arena_t* arena;
} arena_temp_t;

arena_t* arena_create(u64 size);
void*    arena_alloc(arena_t* arena, u64 size);
void     arena_pop(arena_t* arena, u64 size);
void     arena_pop_to(arena_t* arena, u64 pos);
void     arena_destroy(arena_t* arena);

arena_temp_t arena_temp_begin(arena_t* arena);
void         arena_temp_end(arena_temp_t temp);

#endif // BASE_MEM_H
