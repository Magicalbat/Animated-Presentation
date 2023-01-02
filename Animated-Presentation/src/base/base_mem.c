#include "base_mem.h"

#include "os/os.h"

#define COMMIT_PAGES 4

#define COMMIT_SIZE (os_mem_pagesize() * COMMIT_PAGES)

arena_t* arena_create(u64 size) {
    arena_t* arena = os_mem_reserve(size);
    u64 init_commit = MIN(size, COMMIT_SIZE);
    
    os_mem_commit(arena, init_commit);
    
    if (arena) {
        arena->size = size;
        arena->cur = ALIGN_UP_POW2(sizeof(arena_t), 64);
        arena->cur_commit = init_commit;
    } else {
        ASSERT(false, "Arena is NULL");
    }

    return arena;
}
void* arena_alloc(arena_t* arena, u64 size) {
    ASSERT(arena->cur + size < arena->size, "Arena ran out of memory");

    void* out = ((u8*)arena) + arena->cur;
    arena->cur += size;

    if (arena->cur > arena->cur_commit) {
        u64 commit_size = (1 + (arena->cur - arena->cur_commit) / COMMIT_SIZE) * COMMIT_SIZE;
        os_mem_commit(((u8*)arena + arena->cur_commit), commit_size);
        arena->cur_commit += commit_size;
    }

    return out;
}
void arena_pop(arena_t* arena, u64 size) {
    ASSERT(arena->cur - size > 0, "Arena cannot pop any more memory");

    u64 new_pos = arena->cur - size;
    u64 commit_pos = ALIGN_UP_POW2(new_pos, COMMIT_SIZE);

    if (commit_pos < arena->cur_commit) {
        os_mem_decommit((u8*)arena + commit_pos, arena->cur_commit - commit_pos);
        arena->cur_commit = commit_pos;
    }

    arena->cur = new_pos;
}
void arena_pop_to(arena_t* arena, u64 pos) {
    ASSERT(pos <= arena->cur, "Arena cannot pop to that pos");

    arena_pop(arena, arena->cur - pos);
}
void arena_destroy(arena_t* arena) {
    ASSERT(arena != NULL, "Cannot free NULL arena");
    
    os_mem_release(arena, arena->size);
} 

arena_temp_t arena_temp_begin(arena_t* arena) {
    return (arena_temp_t){
        .start_pos = arena->cur,
        .arena = arena
    };
}
void arena_temp_end(arena_temp_t temp) {
    arena_pop_to(temp.arena, temp.start_pos);
}
