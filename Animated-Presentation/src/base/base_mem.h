#ifndef BASE_MEM_H
#define BASE_MEM_H

#include <stdlib.h>
#include <string.h>

#include "base.h"

typedef struct arena {
	uint8_t* data;
	uint64_t size;
	uint64_t cur;
} arena_t;

arena_t* arena_create( uint64_t size                 );
void*    arena_malloc( arena_t* arena, uint64_t size );
void     arena_pop   ( arena_t* arena, uint64_t size );
void     arena_free  ( arena_t* arena                );

typedef struct string8 {
	uint8_t* str;
	uint64_t len;
} string8_t;

string8_t string8_create    ( arena_t* arena, uint64_t len );
string8_t string8_from_cstr ( uint8_t* str );

#endif