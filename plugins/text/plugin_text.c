#include "ap_core.h"
#include "app/app.h"

#include "stb_rect_pack.h"
#include "stb_truetype.h"

/*typedef struct {
    string8 source;
    f64 size;
    b32 is_default;
} pres_font;

typedef struct {
    string8 text;
} pres_text;*/

typedef struct {
    string8 source;
} pres_font;

static draw_rectb* rectb;
static i32 font_img_id;
static stbtt_packedchar glyph_metrics[95];
static struct {
    f32 ascent;
    f32 descent;
    f32 line_gap;
} v_metrics = { 0 };

void test_desc_init(marena* arena, app_app* app, void* custon_data) {
    rectb = draw_rectb_create(arena, app->win, 64, 4, 1);

    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 ttf_file = os_file_read(scratch.arena, STR("res/Hack.ttf"));
    stbtt_pack_range ranges[1] = {
        { 96, 32, NULL, 95, glyph_metrics },
    };

    u32 width = 1024;
    u32 max_height = 1024;
    u8* bitmap = CREATE_ZERO_ARRAY(scratch.arena, u8, width * max_height);

    stbtt_pack_context pc = { 0 };
    stbtt_PackBegin(&pc, bitmap, width, max_height, 0, 1, NULL);
    stbtt_PackSetOversampling(&pc, 1, 1);
    stbtt_PackFontRanges(&pc, ttf_file.str, 0, ranges, 1);
    stbtt_PackEnd(&pc);

    u32 height = 0;
    for (u32 i = 0; i < 95; i++) {
        stbtt_packedchar c = glyph_metrics[i];
        if (c.y1 > height) height = c.y1;
    }

    stbtt_fontinfo info = { 0 };
    stbtt_InitFont(&info, ttf_file.str, stbtt_GetFontOffsetForIndex(ttf_file.str, 0));

    f32 scale = stbtt_ScaleForPixelHeight(&info, 96);
    i32 a, d, l;
    stbtt_GetFontVMetrics(&info, &a, &d, &l);

    v_metrics.ascent = (f32)(a * scale);
    v_metrics.descent = (f32)(d * scale);
    v_metrics.line_gap = (f32)(l * scale);

    draw_rectb_set_filter(rectb, DRAW_FILTER_LINEAR);

    image img = {
        .valid = true,
        .channels = 1,
        .width = width,
        .height = height,
        .data = bitmap
    };

    font_img_id = draw_rectb_add_tex(rectb, img);

    draw_rectb_finalize_textures(rectb);

    marena_temp_end(scratch);
}
void test_desc_destroy(void* custon_data) {
    draw_rectb_destroy(rectb);
}
void test_draw(app_app* app, void* obj) {
    memcpy(rectb->win_mat, app->win_mat, sizeof(app->win_mat));

    string8 str = STR("Hello World!\nHello Again");

    f32 start_x = 50;
    f32 x = start_x;
    f32 start_y = 100;
    f32 y = start_y;
    for (u8* ptr = str.str; ptr < str.str + str.size; ptr += 1) {
        switch (*ptr) {
            case '\n': {
                y += v_metrics.ascent - v_metrics.descent + v_metrics.line_gap;
                x = start_x;
            } break;
            default: break;
        }


        if (*ptr < 32 || *ptr > 127)
            continue;

        stbtt_packedchar pc = glyph_metrics[*ptr - 32];

        vec2 dim = { pc.x1 - pc.x0, pc.y1 - pc.y0 };

        draw_rectb_push_ex(
            rectb,
            (rect){ x + pc.xoff, y + pc.yoff, dim.x, dim.y },
            (vec4d){ 1, 1, 1, 1 },
            font_img_id,
            (rect){ pc.x0, pc.y0, dim.x, dim.y }
        );

        x += pc.xadvance;
    }

    draw_rectb_flush(rectb);
}

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    obj_desc font_desc = {
        .name = STR("font"),
        .obj_size = sizeof(pres_font),

        .desc_init_func = test_desc_init,
        .desc_destroy_func = test_desc_destroy,

        .draw_func = test_draw
    };

    obj_reg_add_desc(arena, app, app->pres->obj_reg, &font_desc);
}
