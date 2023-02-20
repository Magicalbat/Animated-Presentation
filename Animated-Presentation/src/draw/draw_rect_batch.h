#ifndef DRAW_RECT_BATCH_H
#define DRAW_RECT_BATCH_H

#include "base/base.h"
#include "gfx/gfx.h"
#include "parse/parse.h"

typedef struct {
    rect draw_rect;
    vec3 col;
    float tex_id;
    rect tex_rect;
} draw_rectb_rect;

typedef struct {
    b32 active; 

    u32 width;
    u32 height;

    #ifdef AP_OPENGL
        struct { u32 tex; } gl;
    #endif
} draw_rectb_tex;

#define RECTB_MAX_TEXS 16

typedef struct {
    draw_rectb_rect* data;
    u32 capacity;
    u32 size;

    draw_rectb_tex textures[RECTB_MAX_TEXS];

    #ifdef AP_OPENGL
        struct {
            u32 shader_program;
    
            u32 vertex_array;
            u32 vertex_buffer;
            u32 pos_pattern_buffer;
        } gl;
    #endif
} draw_rectb;

draw_rectb* draw_rectb_create(marena* arena, gfx_window* win, u32 capacity);
void draw_rectb_destroy(draw_rectb* batch);

u32 draw_rectb_add_tex(draw_rectb* batch, image img);
u32 draw_rectb_create_tex(draw_rectb* batch, string8 file_path);
void draw_rectb_destroy_tex(draw_rectb* batch, u32 tex_id);

// These functions will draw rects to the screen
void draw_rectb_push_ex(draw_rectb* batch, rect draw_rect, vec3 col, i32 tex_id, rect tex_rect);
void draw_rectb_push(draw_rectb* batch, rect draw_rect, vec3 col);
void draw_rectb_flush(draw_rectb* batch);

#endif // DRAW_RECT_BATCH_H
