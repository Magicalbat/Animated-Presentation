#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <stdlib.h>
#include <string.h>

#include "base.h"

typedef struct arena {
	//uint8_t* data;
	u64 size;
	u64 cur;
    u64 cur_commit;
    // Data is stored in the memory following the struct
} arena_t;

arena_t* arena_create( u64 size                 );
void*    arena_malloc( arena_t* arena, u64 size );
void     arena_pop   ( arena_t* arena, u64 size );
void     arena_free  ( arena_t* arena           );

#endif