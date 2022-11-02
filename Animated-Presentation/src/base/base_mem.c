#include "base_mem.h"
#include "os.h"

// TODO: use commit and decommit 
arena_t* arena_create(uint64_t size) {
	arena_t* out = (arena_t*)malloc(sizeof(arena_t));//malloc(sizeof(arena_t));

	if (out) {
		out->data = (uint8_t*)os_mem_reserve(size);//(uint8_t*)malloc(size);
		os_mem_commit(out->data, size);
		//ASSERT(commit && "Failed to commit arena memory");
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

	os_mem_release(arena->data, arena->size);
	free(arena);
	//free(arena->data);
	//free(arena      );
} 

string8_t string8_create(arena_t* arena, uint64_t len) {
	uint8_t* data = (uint8_t*)arena_malloc(arena, len);
	string8_t out = { data, len };
	return out;
}
string8_t string8_from_cstr(uint8_t* str) {
	return (string8_t){ str, strlen(str) };
}
