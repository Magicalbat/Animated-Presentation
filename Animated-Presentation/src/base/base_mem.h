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
} arena;

typedef struct {
    u64 start_pos;
    arena* arena;
} arena_temp;

arena* arena_create(u64 size);
void*  arena_alloc(arena* arena, u64 size);
void   arena_pop(arena* arena, u64 size);
void   arena_pop_to(arena* arena, u64 pos);
void   arena_destroy(arena* arena);

arena_temp arena_temp_begin(arena* arena);
void       arena_temp_end(arena_temp temp);

#endif // BASE_MEM_H
