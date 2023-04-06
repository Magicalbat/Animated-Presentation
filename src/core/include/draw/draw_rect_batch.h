#ifndef DRAW_RECT_BATCH_H
#define DRAW_RECT_BATCH_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "gfx/gfx.h"
#include "parse/parse.h"
#include "draw/draw.h"

typedef struct {
    rect draw_rect;
    vec4 col;
    rect tex_rect;
} draw_rectb_rect;

typedef struct {
    draw_rectb_rect* data;
    u32 capacity;
    u32 size;

    u32 channels;

    f32* win_mat;

    struct {
        draw_filter_type filter_type;
        marena* arena;
        image* imgs;
    } temp;

    rect* img_rects;
    rect* tex_rects;
    u32 max_textures;
    u32 num_textures;
    rect texture_boundary;

    #ifdef AP_OPENGL
        struct {
            u32 shader_program;
            u32 win_mat_loc;
            u32 texture_loc;

            u32 vertex_array;
            u32 vertex_buffer;
            u32 pos_pattern_buffer;

            u32 texture;
        } gl;
    #endif
} draw_rectb;

AP_EXPORT draw_rectb* draw_rectb_create(marena* arena, f32* win_mat, u32 capacity, u32 max_textures, u32 channels);
AP_EXPORT void draw_rectb_destroy(draw_rectb* batch);

AP_EXPORT void draw_rectb_set_filter(draw_rectb* batch, draw_filter_type filter_type);
AP_EXPORT u32 draw_rectb_add_tex(draw_rectb* batch, image img);
AP_EXPORT u32 draw_rectb_create_tex(draw_rectb* batch, string8 image_file, vec2* dim);
AP_EXPORT u32 draw_rectb_load_tex(draw_rectb* batch, string8 file_path, vec2* dim);
AP_EXPORT void draw_rectb_finalize_textures(draw_rectb* batch);

// These functions will draw rects to the screen
AP_EXPORT void draw_rectb_push_ex(draw_rectb* batch, rect draw_rect, vec4d col, i32 tex_id, rect tex_rect);
AP_EXPORT void draw_rectb_push(draw_rectb* batch, rect draw_rect, vec4d col);
AP_EXPORT void draw_rectb_flush(draw_rectb* batch);

#ifdef __cplusplus
}
#endif

#endif // DRAW_RECT_BATCH_H
