#include "ap_core.h"
#include "app/app.h"

#include "stb_truetype.h"

typedef struct {
    string8 source;
    f64 font_size;

    u32 font_index;
} pres_font;

#define MAX_FONTS 8
#define MAX_FONT_SIZES 8
static struct {
    stbtt_fontinfo font_info;
    f32 font_sizes[MAX_FONT_SIZES];

    string8 source;
    u64 reg_id;
    b32 loaded;
} fonts[MAX_FONTS] = { };

void font_obj_default(marena* arena, app_app* app, void* obj) {
    pres_font* font = (pres_font*)obj;

    *font = (pres_font){
        .font_size = 12
    };
}

void font_obj_init(marena* arena, app_app* app, void* obj) {
    pres_font* font = (pres_font*)obj;

    i32 font_index = -1;
    for (i32 i = 0; i < MAX_FONTS; i++) {
        if (str8_equals(font->source, fonts[i].source)) {
            b32 size_found = false;
            for (i32 j = 0; j < MAX_FONT_SIZES; j++) {
                if (fonts[i].font_sizes[j] == font->font_size) {
                    size_found = true;
                    break;
                }
            }

            if (!size_found) {
                for (i32 j = 0; j < MAX_FONT_SIZES; j++) {
                    if (fonts[i].font_sizes[j] == 0) {
                        fonts[i].font_sizes[j] = font->font_size;
                        break;
                    }
                }
            }
            
            font_index = i;
            break;
        }
    }
    
    if (font_index == -1) {
        for (i32 i = 0; i < MAX_FONTS; i++) {
            if (!fonts[i].loaded) {
                font_index = i;

                fonts[i].reg_id = str8_reg_push(app->temp.arena, &app->temp.file_reg, font->source);
                
                fonts[i].source = font->source;
                fonts[i].font_sizes[0] = (f32)font->font_size;
                fonts[i].loaded = true;

                break;
            }
        }
    }

    if (font_index == -1){
        log_error("Text plugin out of font indices");
        return;
    }

    font->font_index = font_index;
}

static u32 test_img;

static b32 first_file_func = true;
void font_obj_file(marena* arena, app_app* app, void* obj) {
    if (first_file_func) {
        first_file_func = false;

        for (u32 i = 0; i < MAX_FONTS; i++) {
            if (fonts[i].source.size == 0)
                continue;

            string8 file_str = str8_reg_get(&app->temp.file_reg, fonts[i].reg_id);
            
            // TODO: finish font loading
            // example: https://gist.github.com/vassvik/f442a4cc6127bc7967c583a12b148ac9

            stbtt_packedchar glyph_metrics[95];
            stbtt_pack_range ranges[1] = {
                { 48, 32, NULL, 95, glyph_metrics }
            };

            u32 width = 1024;
            u32 height = 256;
            u8* bitmap = CREATE_ZERO_ARRAY(app->temp.arena, u8, width * height);

            stbtt_pack_context pc;
            stbtt_PackBegin(&pc, bitmap, width, height, 0, 1, NULL);   
            stbtt_PackSetOversampling(&pc, 1, 1);
            stbtt_PackFontRanges(&pc, file_str.str, 0, ranges, 1);
            stbtt_PackEnd(&pc);

            image img = {
                .channels = 4,
                .width = width,
                .height = height,
                .valid = true,
                .data = CREATE_ZERO_ARRAY(app->temp.arena, u8, width * height * 4)
            };

            u32 i = 0;
            for (u32 x = 0; x < width; x++) {
                for (u32 y = 0; y < height; y++) {
                    img.data[i++] = bitmap[x + y * width];
                    img.data[i++] = bitmap[x + y * width];
                    img.data[i++] = bitmap[x + y * width];
                    img.data[i++] = 255;
                }
            }

            test_img = draw_rectb_add_tex(app->rectb, img);
        }
    }
}

void test_draw(app_app* app, void* obj) {
    draw_rectb_push_ex(
        app->rectb,
        (rect){ 0, 0, 256, 256 },
        (vec4d){ 1, 1, 1, 1 },
        test_img, (rect){ 0 }
    );
}

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    obj_desc font_desc = {
        .name = STR("font"),
        .obj_size = sizeof(pres_font),

        .default_func = font_obj_default,
        .init_func = font_obj_init,
        .file_func = font_obj_file,
        .draw_func = test_draw,

        .fields = {
            { STR("source"), FIELD_STR8, offsetof(pres_font, source) },
            { STR("font_size"), FIELD_F64, offsetof(pres_font, font_size) },
        }
    };

    obj_reg_add_desc(app->pres->obj_reg, &font_desc);
}