#ifndef DRAW_H
#define DRAW_H

#include "base/base.h"
#include "gfx/gfx.h"

#ifdef AP_OPENGL
#include "gfx/opengl/opengl.h"
#endif

// TODO: benchmark this vs instancing
typedef struct {
    vec2_t bottom_left;
    vec2_t top_left;
    vec2_t top_right;
    vec2_t bottom_right;
} draw_rect_t;

typedef struct {
    draw_rect_t* data;
    u64 capacity;
    u64 size;

    #ifdef AP_OPENGL
    struct {
        u32 vert_array;
        u32 vert_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_rect_batch_t;

draw_rect_batch_t* draw_rect_batch_create(arena_t* arena, u64 capacity);
void               draw_rect_batch_destroy(arena_t* arena, u64 capacity);

// These will draw rects to the screen
void draw_rect_batch_push(draw_rect_batch_t* batch, draw_rect_t rect);
void draw_rect_batch_flush(draw_rect_batch_t* batch);

#endif // DRAW_H
