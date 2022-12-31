#ifndef DRAW_RECT_BATCH_H
#define DRAW_RECT_BATCH_H

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    rect_t rect;
    vec3_t col;
} draw_rect_t;

typedef struct {
    draw_rect_t* data;
    u64 capacity;
    u32 size;

    #ifdef AP_OPENGL
    struct {
        u32 shader_program;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 pos_pattern_buffer;
    } gl;
    #endif
} draw_rectb_t;

draw_rectb_t* draw_rectb_create(arena_t* arena, gfx_window_t* win, u64 capacity);
void          draw_rectb_destroy(draw_rectb_t* batch);

// These will draw rects to the screen
void draw_rectb_push(draw_rectb_t* batch, rect_t draw_rect, vec3_t col);

void draw_rectb_flush(draw_rectb_t* batch);

#endif // DRAW_RECT_BATCH_H
