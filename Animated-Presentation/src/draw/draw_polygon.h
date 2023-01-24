#ifndef DRAW_POLYGON_H
#define DRAW_POLYGON_H

#include "base/base.h"
#include "gfx/gfx.h"

typedef struct {
    u32 max_verts;

    vec2* verts;
    u32* indices;

    #if defined (AP_OPENGL)
    struct {
        u32 shader_program;
        u32 col_loc;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_polygon;

typedef struct vec2_node {
    vec2 v;
    struct vec2_node* next;
} vec2_node;

typedef struct {
    vec2_node* first;
    vec2_node* last;
    u32 size;
} vec2_list;

typedef struct {
    vec2* data;
    u32 size;
} vec2_arr;

draw_polygon* draw_poly_create(arena* arena, gfx_window* win, u32 max_verts);
void          draw_poly_destroy(draw_polygon* poly);

void draw_poly_conv_list(draw_polygon* poly, vec3 col, vec2_list list);
void draw_poly_conv_arr (draw_polygon* poly, vec3 col, vec2_arr arr);

void draw_poly_list(draw_polygon* poly, vec3 col, vec2_list list);
void draw_poly_arr (draw_polygon* poly, vec3 col, vec2_arr arr);

#endif // DRAW_POLYGON_H
