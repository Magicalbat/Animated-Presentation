#ifndef DRAW_RECT_BATCH_H
#define DRAW_RECT_BATCH_H

#include "base/base.h"
#include "gfx/gfx.h"

typedef enum {
    RECTB_COLOR,
    RECTB_TEXTURE,
    RECTB_BOTH,
} draw_rectb_type;

typedef struct {
    u8* data;
    u32 capacity;
    u32 size;

    draw_rectb_type type;

    #ifdef AP_OPENGL
        struct {
            u32 texture;
    
            u32 shader_program;
    
            u32 vertex_array;
            u32 vertex_buffer;
            u32 pos_pattern_buffer;
        } gl;
    #endif
} draw_rectb;

draw_rectb* draw_rectb_create_ex(arena* arena, gfx_window* win, u32 capacity, draw_rectb_type type, string8 texture_path);
#define draw_rectb_create(arena, win, capacity) draw_rectb_create_ex(arena, win, capacity, RECTB_COLOR, (string8){ .size=0 })
void draw_rectb_destroy(draw_rectb* batch);

// These functions will draw rects to the screen
void draw_rectb_push_col(draw_rectb* batch, rect draw_rect, vec3 col);
void draw_rectb_push_tex(draw_rectb* batch, rect draw_rect, rect tex_rect);
void draw_rectb_push_both(draw_rectb* batch, rect draw_rect, vec3 col, rect tex_rect);
void draw_rectb_flush(draw_rectb* batch);

#endif // DRAW_RECT_BATCH_H
