#ifndef DRAW_POLYGON_H
#define DRAW_POLYGON_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    u32 max_verts;

    vec2* verts;
    u32* indices;

    f32* win_mat;

    #if defined (AP_OPENGL)
    struct {
        u32 shader_program;
        u32 win_mat_loc;
        u32 col_loc;
        u32 offset_loc;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_polygon;

typedef struct vec2d_node {
    vec2d v;
    struct vec2d_node* next;
} vec2d_node;

typedef struct {
    vec2d_node* first;
    vec2d_node* last;
    u32 size;
} vec2d_list;

typedef struct {
    u64 size;
    vec2d* data;
} vec2d_arr;

AP_EXPORT draw_polygon* draw_poly_create(marena* arena, f32* win_mat, u32 max_verts);
AP_EXPORT void          draw_poly_destroy(draw_polygon* poly);

AP_EXPORT void draw_poly_conv_list(draw_polygon* poly, vec4d col, vec2d offset, vec2d_list list);
AP_EXPORT void draw_poly_conv_arr (draw_polygon* poly, vec4d col, vec2d offset, vec2d_arr arr);

// TODO: write these two functiosn
// void draw_poly_list(draw_polygon* poly, vec3 col, vec2_list list);
// void draw_poly_arr (draw_polygon* poly, vec3 col, vec2_arr arr);

#ifdef __cplusplus
}
#endif

#endif // DRAW_POLYGON_H
