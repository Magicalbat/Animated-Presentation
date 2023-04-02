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

void test_desc_init(marena* arena, app_app* app, void* custon_data) {
    rectb = draw_rectb_create(arena, app->win, 64, 4, 1);

    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 ttf_file = os_file_read(scratch.arena, STR("res/Hack.ttf"));
    stbtt_packedchar glyph_metrics[95];
    stbtt_pack_range ranges[1] = {
        { 96, 32, NULL, 95, glyph_metrics },
    };

    u32 width = 1024;
    u32 height = 1024;
    u8* bitmap = CREATE_ZERO_ARRAY(scratch.arena, u8, width * height);

    stbtt_pack_context pc = { 0 };
    stbtt_PackBegin(&pc, bitmap, width, height, 0, 1, NULL);
    stbtt_PackSetOversampling(&pc, 1, 1);
    stbtt_PackFontRanges(&pc, ttf_file.str, 0, ranges, 1);
    stbtt_PackEnd(&pc);

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

    draw_rectb_push_ex(rectb, (rect){ 5, 5, 250, 150 }, (vec4d){ 1, 1, 1, 1 }, font_img_id, (rect){ 0 });
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
