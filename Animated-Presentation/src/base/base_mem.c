#include "base_mem.h"
#include "os.h"

#define INIT_COMMIT_PAGES 1

arena_t* arena_create(uint64_t size) {
	arena_t* arena = os_mem_reserve(size);
    uint64_t init_commit = os_mem_pagesize() * INIT_COMMIT_PAGES;
    
    os_mem_commit(arena, init_commit);
    
	if (arena) {
		arena->size = size;
		arena->cur = 24;
        arena->cur_commit = init_commit;
	} else {
		ASSERT(false, "Arena is NULL");
	}

	return arena;
}
void* arena_malloc(arena_t* arena, uint64_t size) {
	ASSERT(arena->cur + size < arena->size, "Arena ran out of memory");

	arena->cur += size;

    if (arena->cur > arena->cur_commit) {
        uint64_t commit_size = (1 + (arena->cur - arena->cur_commit) / os_mem_pagesize()) * os_mem_pagesize();
        os_mem_commit((void*)((uint64_t)arena + arena->cur_commit), commit_size);
        arena->cur_commit += commit_size;
    }

	return (void*)((uint64_t)arena + arena->cur - size);
}
void arena_pop(arena_t* arena, uint64_t size) {
	ASSERT(arena->cur - size > 0, "Arena cannot pop any more memory");

    uint64_t new_pos = arena->cur - size;
    uint64_t commit_pos = ALIGN_UP_POW2(new_pos, os_mem_pagesize());

    if (commit_pos < arena->cur_commit) {
        os_mem_decommit((void*)((uint64_t)arena + commit_pos), arena->cur_commit - commit_pos);
        arena->cur_commit = commit_pos;
    }

    arena->cur = new_pos;
}
void arena_free(arena_t* arena) {
	ASSERT(arena != NULL, "Cannot free NULL arena");
    
    os_mem_release(arena, arena->size);
} 

string8_t string8_create(arena_t* arena, uint64_t len) {
	uint8_t* data = (uint8_t*)arena_malloc(arena, len);
	string8_t out = { data, len };
	return out;
}
string8_t string8_from_cstr(uint8_t* str) {
	return (string8_t){ str, strlen(str) };
}
