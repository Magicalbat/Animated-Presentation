#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    vec2 pos;
    vec3 col;
} cb_vertex;

typedef struct {
    cbezier bezier;

    cb_vertex* verts;
    u32 capacity;
    u32 size;

    #if defined(AP_OPENGL)
    struct {
        u32 shader_program;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_cbezier;

draw_cbezier* draw_cbezier_create(arena* arena, u32 capacity);
void          draw_cbezier_destroy(draw_cbezier* draw_cb);

void draw_cbezier_push(draw_cbezier* draw_cb, vec3 col);
void draw_cbezier_push_grad(draw_cbeier* draw_cb, vec3 start_col, vec3 end_col);
void draw_cbezier_flush(draw_cbezier* draw_cb);

#endif // DRAW_BEZIER_H