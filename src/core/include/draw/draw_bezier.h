#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    vec2 pos;
    vec3 col;
} cb_vertex;

typedef struct {
    gfx_window* win;

    u32* indices;
    cb_vertex* vertices;
    u32 capacity;

    u32 index_pos;
    u32 vertex_pos;

    #if defined(AP_OPENGL)
    struct {
        u32 shader_program;
        u32 win_mat_loc;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_cbezier;

draw_cbezier* draw_cbezier_create(marena* arena, gfx_window* win, u32 capacity);
void          draw_cbezier_destroy(draw_cbezier* draw_cb);

//void draw_cbezier_push(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec3 col);
void draw_cbezier_push_grad(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec3 start_col, vec3 end_col);
void draw_cbezier_push(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec3 col);

void draw_cbezier_flush(draw_cbezier* draw_cb);

#ifdef __cplusplus
}
#endif

#endif // DRAW_BEZIER_H
