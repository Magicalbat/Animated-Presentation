#ifndef DRAW_BEZIER_H
#define DRAW_BEZIER_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    vec2 pos;
    vec4 col;
} cb_vertex;

typedef struct {
    u32* indices;
    cb_vertex* vertices;
    u32 capacity;

    u32 index_pos;
    u32 vertex_pos;

    f32* win_mat;

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

AP_EXPORT draw_cbezier* draw_cbezier_create(marena* arena, f32* win_mat, u32 capacity);
AP_EXPORT void          draw_cbezier_destroy(draw_cbezier* draw_cb);

AP_EXPORT void draw_cbezier_push_grad(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec4d start_col, vec4d end_col);
AP_EXPORT void draw_cbezier_push(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec4d col);

AP_EXPORT void draw_cbezier_flush(draw_cbezier* draw_cb);

#ifdef __cplusplus
}
#endif

#endif // DRAW_BEZIER_H
