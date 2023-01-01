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

arena_t* arena_create(u64 size);
void*    arena_alloc(arena_t* arena, u64 size);
void     arena_pop(arena_t* arena, u64 size);
void     arena_pop_to(arena_t* arena, u64 pos);
void     arena_destroy(arena_t* arena);

#endif // BASE_MEM_H
