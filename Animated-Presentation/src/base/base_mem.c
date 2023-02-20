#include "base_mem.h"

#include "os/os.h"

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static u32 round_pow2(u32 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    
    return v;
}

typedef struct {
    u64 max_size;
    u32 block_size;
} init_data;

static init_data init_common(const arena_desc* desc) {
    init_data out = { 0 };
    
    u32 page_size = (u32)os_mem_pagesize();
    
    out.max_size = ALIGN_UP_POW2(desc->desired_max_size, page_size);
    u32 desired_block_size = desc->desired_block_size == 0 ? 
        ALIGN_UP_POW2(out.max_size / 8, page_size) : desc->desired_block_size;
    desired_block_size = ALIGN_UP_POW2(desired_block_size, page_size);
    
    out.block_size = round_pow2(desired_block_size);
    
    return out;
}

#ifdef __EMSCRIPTEN__

arena* arena_create(const arena_desc* desc) {
    init_data init_data = init_common(desc);

    arena* out = (arena*)malloc(sizeof(arena));

    if (out == NULL) {
        log_error("Failed to malloc initial memory for arena");

        return NULL;
    }
    
    out->pos = 0;
    out->size = init_data.max_size;
    out->block_size = init_data.block_size;

    out->malloc_backend.cur_node = (arena_malloc_node*)malloc(sizeof(arena_malloc_node));
    *out->malloc_backend.cur_node = (arena_malloc_node){
        .prev = NULL,
        .size = out->block_size,
        .pos = 0,
        .data = (u8*)malloc(out->block_size)
    };

    return out;
}
void arena_destroy(arena* arena) {
    arena_malloc_node* node = arena->malloc_backend.cur_node;
    while (node != NULL) {
        free(node->data);

        arena_malloc_node* temp = node;
        node = node->prev;
        free(temp);
    }
    
    free(arena);
}

void* arena_push(arena* arena, u64 size) {
    if (arena->pos + size > arena->size) {
        log_error("Arena ran out of memory");
        return NULL;
    }

    arena_malloc_node* node = arena->malloc_backend.cur_node;

    u64 pos = node->pos;
    arena->pos += size;

    if (arena->pos >= node->size) {
        
        u64 unclamped_node_size = ALIGN_UP_POW2(size, arena->block_size);
        u64 max_node_size = arena->size - arena->pos;
        u64 node_size = MIN(unclamped_node_size, max_node_size);
        
        arena_malloc_node* new_node = (arena_malloc_node*)malloc(sizeof(arena_malloc_node));
        u8* data = (u8*)malloc(node_size);

        if (new_node == NULL || data == NULL) {
            if (new_node != NULL) { free(new_node); }
            if (data != NULL) { free(data); }
            
            log_error("Failed to malloc new node");
            return NULL;
        }

        new_node->pos = size;
        new_node->size = node_size;
        new_node->data = data;
        
        new_node->prev = node;
        arena->malloc_backend.cur_node = new_node;

        return (void*)(new_node->data);
    }
    
    void* out = (void*)((u8*)node->data + pos);
    node->pos += size;

    return out;
}

void arena_pop(arena* arena, u64 size) {
    if (size > arena->pos) {
        log_error("Attempted to pop too much memory");
    }
    
    u64 size_left = size;
    arena_malloc_node* node = arena->malloc_backend.cur_node;

    while (size_left > node->pos) {
        size_left -= node->pos;
        
        arena_malloc_node* temp = node;
        node = node->prev;

        free(temp->data);
        free(temp);
    }

    node->pos -= size_left;
    arena->pos -= size_left;
}

void arena_reset(arena* arena) {
    arena_pop_to(arena, 0);
}

#else

static const u32 ARENA_MIN_POS = ALIGN_UP_POW2(sizeof(arena), 64);

arena* arena_create(const arena_desc* desc) {
    init_data init_data = init_common(desc);
    
    arena* out = os_mem_reserve(init_data.max_size);

    if (!os_mem_commit(out, init_data.block_size)) {
        log_error("Failed to commit initial memory for arena");

        return NULL;
    }

    out->pos = ARENA_MIN_POS;
    out->size = init_data.max_size;
    out->block_size = init_data.block_size;
    out->reserve_backend.commit_pos = init_data.block_size;

    return out;
}
void arena_destroy(arena* arena) {
    os_mem_release(arena, arena->size);
}

void* arena_push(arena* arena, u64 size) {
    if (arena->pos + size > arena->size) {
        log_error("Arena ran out of memory");

        return NULL;
    }

    void* out = (void*)((u8*)arena + arena->pos);
    arena->pos += size;

    u64 commit_pos = arena->reserve_backend.commit_pos;
    if (arena->pos > commit_pos) {
        u64 commit_unclamped = ALIGN_UP_POW2(arena->pos, arena->block_size);
        u64 new_commit_pos = MIN(commit_unclamped, arena->size);
        u64 commit_size = new_commit_pos - commit_pos;
        
        if (!os_mem_commit((void*)((u8*)arena + commit_pos), commit_size)) {
            log_error("Failed to commit memory for arena");

            return NULL;
        }

        arena->reserve_backend.commit_pos = new_commit_pos;
    }

    return out;
}

void arena_pop(arena* arena, u64 size) {
    if (size > arena->pos - ARENA_MIN_POS) {
        log_error("Attempted to pop too much memory");

        return;
    }

    arena->pos = MAX(ARENA_MIN_POS, arena->pos - size);

    u64 new_commit = MIN(arena->size, ALIGN_UP_POW2(arena->pos, arena->block_size));
    u64 commit_pos = arena->reserve_backend.commit_pos;

    if (new_commit < commit_pos) {
        u64 decommit_size = commit_pos - new_commit;
        os_mem_decommit((void*)((u8*)arena + new_commit), decommit_size);
        arena->reserve_backend.commit_pos = new_commit;
    }
}

void arena_reset(arena* arena) {
    arena_pop_to(arena, ARENA_MIN_POS);
}

#endif

void* arena_push_zero(arena* arena, u64 size) {
    u8* out = arena_push(arena, size);
    memset(out, 0, size);
    
    return (void*)out;
}

void arena_pop_to(arena* arena, u64 pos) {
    arena_pop(arena, arena->pos - pos);
}

arena_temp arena_temp_begin(arena* arena) {
    return (arena_temp){
        .arena = arena,
        .pos = arena->pos
    };
}
void arena_temp_end(arena_temp temp) {
    arena_pop_to(temp.arena, temp.pos);
}

#ifndef AP_SCRATCH_COUNT
#   define AP_SCRATCH_COUNT 2
#endif

AP_THREAD_VAR static arena_desc scratch_desc = {
    .desired_max_size = MiB(64),
    .desired_block_size = KiB(128)
};
AP_THREAD_VAR static arena* scratch_arenas[AP_SCRATCH_COUNT] = { 0 };

void arena_scratch_set_desc(arena_desc* desc) {
    if (scratch_arenas[0] == NULL)
        scratch_desc = *desc;
}
arena_temp arena_scratch_get(arena** conflicts, u32 num_conflicts) {
    if (scratch_arenas[0] == NULL) {
        for (u32 i = 0; i < AP_SCRATCH_COUNT; i++) {
            scratch_arenas[i] = arena_create(&scratch_desc);
        }
    }

    arena_temp out = { 0 };

    for (u32 i = 0; i < AP_SCRATCH_COUNT; i++) {
        arena* arena = scratch_arenas[i];

        b32 in_conflict = false;
        for (u32 j = 0; j < num_conflicts; j++) {
            if (arena == conflicts[j]) {
                in_conflict = true;
                break;
            }
        }
        if (in_conflict) { continue; }

        out = arena_temp_begin(arena);
    }

    return out;
}
void arena_scratch_release(arena_temp scratch) {
    arena_temp_end(scratch);
}
