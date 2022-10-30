#include "base_mem.h"

arena_t* arena_create(uint64_t size) {
	arena_t* out = malloc(sizeof(arena_t));

	if (out) {
		out->data = (uint8_t*)malloc(size);
		if (out->data)
			memset(out->data, 0, size);
	
		out->size = size;
		out->cur = 0;
	} else {
		ASSERT(false && "Arena is NULL");
	}

	return out;
}
void* arena_malloc(arena_t* arena, uint64_t size) {
	ASSERT(arena->cur + size < arena->size && "Arena ran out of memory");

	arena->cur += size;
	return (void*)(arena->data + arena->cur - size);
}
void arena_pop(arena_t* arena, uint64_t size) {
	ASSERT(arena->cur - size > 0 && "Arena cannot pop any more memory");

	arena->cur -= size;
	memset((void*)(arena->data + arena->cur), 0, size);
}
void arena_free(arena_t* arena) {
	ASSERT(arena != NULL       && "Cannot free NULL arena"     );
	ASSERT(arena->data != NULL && "Cannot free NULL arena data");

	free(arena->data);
	free(arena      );
} 
