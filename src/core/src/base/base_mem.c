#include "base/base_mem.h"

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
    u32 align;
} init_data;

static init_data init_common(const marena_desc* desc) {
    init_data out = { 0 };
    
    u32 page_size = (u32)os_mem_pagesize();
    
    out.max_size = ALIGN_UP_POW2(desc->desired_max_size, page_size);
    u32 desired_block_size = desc->desired_block_size == 0 ? 
        ALIGN_UP_POW2(out.max_size / 8, page_size) : desc->desired_block_size;
    desired_block_size = ALIGN_UP_POW2(desired_block_size, page_size);
    
    out.block_size = round_pow2(desired_block_size);
    out.align = desc->align == 0 ? (sizeof(void*)) : desc->align;
    
    return out;
}


#if __EMSCRIPTEN__

marena* marena_create(const marena_desc* desc) {
    init_data init_data = init_common(desc);

    marena* out = (marena*)malloc(sizeof(marena));

    if (out == NULL) {
        log_error("Failed to malloc initial memory for arena");

        return NULL;
    }
    
    out->pos = 0;
    out->size = init_data.max_size;
    out->block_size = init_data.block_size;
    out->align = init_data.align;

    out->malloc_backend.cur_node = (marena_malloc_node*)malloc(sizeof(marena_malloc_node));
    *out->malloc_backend.cur_node = (marena_malloc_node){
        .prev = NULL,
        .size = out->block_size,
        .pos = 0,
        .data = (u8*)malloc(out->block_size)
    };

    return out;
}
void marena_destroy(marena* arena) {
    marena_malloc_node* node = arena->malloc_backend.cur_node;
    while (node != NULL) {
        free(node->data);

        marena_malloc_node* temp = node;
        node = node->prev;
        free(temp);
    }
    
    free(arena);
}

void* marena_push(marena* arena, u64 size) {
    if (arena->pos + size > arena->size) {
        log_error("Arena ran out of memory");
        return NULL;
    }

    marena_malloc_node* node = arena->malloc_backend.cur_node;

    u64 pos_aligned = ALIGN_UP_POW2(node->pos, arena->align);
    u32 diff = pos_aligned - node->pos;
    arena->pos += diff + size;

    if (arena->pos >= node->size) {
        u64 unclamped_node_size = ALIGN_UP_POW2(size, arena->block_size);
        u64 max_node_size = arena->size - arena->pos;
        u64 node_size = MIN(unclamped_node_size, max_node_size);
        
        marena_malloc_node* new_node = (marena_malloc_node*)malloc(sizeof(marena_malloc_node));
        u8* data = (u8*)malloc(node_size);

        if (new_node == NULL || data == NULL) {
            if (new_node != NULL) { free(new_node); }
            if (data != NULL) { free(data); }
            
            return NULL;
        }

        new_node->pos = size;
        new_node->size = node_size;
        new_node->data = data;
        
        new_node->prev = node;
        arena->malloc_backend.cur_node = new_node;

        return (void*)(new_node->data);
    }
    
    void* out = (void*)((u8*)node->data + pos_aligned);
    node->pos = pos_aligned + size;

    return out;
}

void marena_pop(marena* arena, u64 size) {
    if (size > arena->pos) {
        log_error("Attempted to pop too much memory");
    }
    
    u64 size_left = size;
    marena_malloc_node* node = arena->malloc_backend.cur_node;

    while (size_left > node->pos) {
        size_left -= node->pos;
        
        marena_malloc_node* temp = node;
        node = node->prev;

        free(temp->data);
        free(temp);
    }

    arena->malloc_backend.cur_node = node;
    node->pos -= size_left;
    arena->pos -= size;
}

void marena_reset(marena* arena) {
    marena_pop_to(arena, 0);
}

#else

#define ARENA_MIN_POS ALIGN_UP_POW2(sizeof(marena), 64)

marena* marena_create(const marena_desc* desc) {
    init_data init_data = init_common(desc);
    
    marena* out = os_mem_reserve(init_data.max_size);

    if (!os_mem_commit(out, init_data.block_size)) {
        log_error("Failed to commit initial memory for arena");

        return NULL;
    }

    out->pos = ARENA_MIN_POS;
    out->size = init_data.max_size;
    out->block_size = init_data.block_size;
    out->align = init_data.align;
    out->reserve_backend.commit_pos = init_data.block_size;

    return out;
}
void marena_destroy(marena* arena) {
    os_mem_release(arena, arena->size);
}

void* marena_push(marena* arena, u64 size) {
    if (arena->pos + size > arena->size) {
        log_error("Arena ran out of memory");

        return NULL;
    }

    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, arena->align);
    void* out = (void*)((u8*)arena + pos_aligned);
    arena->pos = pos_aligned + size;

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

void marena_pop(marena* arena, u64 size) {
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

void marena_reset(marena* arena) {
    marena_pop_to(arena, ARENA_MIN_POS);
}

#endif

void* marena_push_zero(marena* arena, u64 size) {
    u8* out = marena_push(arena, size);
    memset(out, 0, size);

    return (void*)out;
}

void* marena_push_noalign(marena* arena, u64 size) {
    u32 real_align = arena->align;

    arena->align = 1;
    void* out = marena_push(arena, size);
    arena->align = real_align;

    return out;
}
void* marena_push_zero_noalign(marena* arena, u64 size) {
    u32 real_align = arena->align;

    arena->align = 1;
    void* out = marena_push_zero(arena, size);
    arena->align = real_align;

    return out;
}

void marena_pop_to(marena* arena, u64 pos) {
    marena_pop(arena, arena->pos - pos);
}

marena_temp marena_temp_begin(marena* arena) {
    return (marena_temp){
        .arena = arena,
        .pos = arena->pos
    };
}
void marena_temp_end(marena_temp temp) {
    marena_pop_to(temp.arena, temp.pos);
}

#ifndef AP_SCRATCH_COUNT
#   define AP_SCRATCH_COUNT 2
#endif

static AP_THREAD_VAR marena_desc scratch_desc = {
    .desired_max_size = MiB(64),
    .desired_block_size = KiB(128)
};
static AP_THREAD_VAR marena* scratch_arenas[AP_SCRATCH_COUNT] = { 0 };

void marena_scratch_set_desc(marena_desc* desc) {
    if (scratch_arenas[0] == NULL)
        scratch_desc = *desc;
}
marena_temp marena_scratch_get(marena** conflicts, u32 num_conflicts) {
    if (scratch_arenas[0] == NULL) {
        for (u32 i = 0; i < AP_SCRATCH_COUNT; i++) {
            scratch_arenas[i] = marena_create(&scratch_desc);
        }
    }

    marena_temp out = { 0 };

    for (u32 i = 0; i < AP_SCRATCH_COUNT; i++) {
        marena* arena = scratch_arenas[i];

        b32 in_conflict = false;
        for (u32 j = 0; j < num_conflicts; j++) {
            if (arena == conflicts[j]) {
                in_conflict = true;
                break;
            }
        }
        if (in_conflict) { continue; }

        out = marena_temp_begin(arena);
    }

    return out;
}
void marena_scratch_release(marena_temp scratch) {
    marena_temp_end(scratch);
}
