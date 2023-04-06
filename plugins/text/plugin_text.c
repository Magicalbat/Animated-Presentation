#include "ap_core.h"
#include "app/app.h"

#include "stb_rect_pack.h"
#include "stb_truetype.h"

typedef struct {
    u32 face;
    u32 font;
} font_ref;

typedef struct {
    string8 source;
    struct { u64 size; f64* data; } sizes;
    b32 is_default;
} pres_font;

typedef struct {
    string8 text;
    string8 font_name;
    f64 font_size;
    f64 x, y;
    vec4d col;

    font_ref font;
} pres_text;

#define MAX_FACES 8
#define MAX_FONTS 8
#define NUM_CHARS 95
#define FIRST_CHAR 32

typedef struct {
    f32 size;

    f32 ascent;
    f32 descent;
    f32 line_gap;
    f32 tab_size;

    stbtt_packedchar* glyph_metrics;
} font_font;

typedef struct {
    string8 source;
    string8 name;

    u64 file_reg_id;

    u32 num_fonts;
    font_font fonts[MAX_FONTS];
} font_face;

static draw_rectb* rectb = NULL;
static i32 font_img_id = -1;
static u32 num_faces = 0;
static font_face faces[MAX_FACES] = { 0 };
static font_ref default_font = { 0 };

void font_desc_init(marena* arena, app_app* app) {
    rectb = draw_rectb_create(arena, app->win_mat, 64, 2, 1);
    draw_rectb_set_filter(rectb, DRAW_FILTER_LINEAR);
}
void font_desc_destroy() {
    draw_rectb_destroy(rectb);
}

void font_default(marena* arena, app_app* app, void* obj) {
    pres_font* pfont = (pres_font*)obj;

    *pfont = (pres_font){ 0 };
}
void font_init(marena* arena, app_app* app, void* obj) {
    pres_font* pfont = (pres_font*)obj;

    i64 face_index = -1;
    for (u32 i = 0; i < MAX_FACES; i++) {
        if (str8_equals(pfont->source, faces[i].source)) {
            face_index = i;
            break;
        }
    }

    if (face_index == -1) {
        face_index = num_faces++;
        if (face_index >= MAX_FACES) {
            log_errorf("Cannot find face slot for font at \"%.*s\"", (int)pfont->source.size, pfont->source.str);
            return;
        }

        faces[face_index].source = pfont->source;

        faces[face_index].file_reg_id = str8_reg_push(app->temp.arena, &app->temp.file_reg, pfont->source);

        string8 name = pfont->source;
        u64 last_slash = 0;
        u64 first_period = name.size;

        for (u64 i = 0; i < name.size; i++) {
            if (name.str[i] == '/' || name.str[i] == '\\') {
                last_slash = i;
            }

            if (name.str[i] == '.' && first_period == name.size) {
                first_period = i;
            }
        }

        name = str8_substr(name, last_slash + 1, first_period);
        faces[face_index].name = name;
    }

    font_face* face = &faces[face_index];

    for (u64 i = 0; i < pfont->sizes.size; i++) {
        f64 size = pfont->sizes.data[i];

        i64 font_index = -1;
        for (u32 j = 0; j < MAX_FONTS; j++) {
            if (face->fonts[j].size == size) {
                font_index = j;
                break;
            }
        }
        if (font_index == -1) {
            font_index = face->num_fonts++;
        }

        if (font_index >= MAX_FONTS) {
            log_errorf("Out of fonts for font \"%.*s\"", (int)face->name.size, face->name.str);
            return;
        }
        
        if (pfont->is_default && i == 0) {
            default_font.face = face_index;
            default_font.font = font_index;
        }

        face->fonts[font_index].size = size;
    }
}

static b32 first_file_call = true;
void font_file(marena* arena, app_app* app, void* obj) {
    if (!first_file_call)
        return;

    first_file_call = false;

    marena_temp scratch = marena_scratch_get(&arena, 1);

    u32 width = 1024;
    u32 max_height = 1024;
    u8* bitmap = CREATE_ZERO_ARRAY(scratch.arena, u8, width * max_height);
    
    u32 height = 0;

    stbtt_pack_context pc = { 0 };
    stbtt_PackBegin(&pc, bitmap, width, max_height, 0, 1, NULL);
    stbtt_PackSetOversampling(&pc, 1, 1);

    for (u32 i = 0; i < num_faces; i++) {
        string8 ttf_file = str8_reg_get(&app->temp.file_reg, faces[i].file_reg_id);

        stbtt_pack_range* ranges = CREATE_ARRAY(scratch.arena, stbtt_pack_range, faces[i].num_fonts);

        for (u32 j = 0; j < faces[i].num_fonts; j++) {
            faces[i].fonts[j].glyph_metrics = CREATE_ARRAY(arena, stbtt_packedchar, NUM_CHARS);

            ranges[j] = (stbtt_pack_range){
                .font_size = faces[i].fonts[j].size,
                .first_unicode_codepoint_in_range = FIRST_CHAR,
                .num_chars = NUM_CHARS,
                .chardata_for_range = faces[i].fonts[j].glyph_metrics
            };
        }
        
        stbtt_PackFontRanges(&pc, ttf_file.str, 0, ranges, faces[i].num_fonts);

        for (u32 j = 0; j < faces[i].num_fonts; j++) {
            for (u32 k = 0; k < NUM_CHARS; k++) {
                if (faces[i].fonts[j].glyph_metrics[k].y1 > height)
                    height = (u32)faces[i].fonts[j].glyph_metrics[k].y1;
            }
        }

        stbtt_fontinfo info = { 0 };
        stbtt_InitFont(&info, ttf_file.str, stbtt_GetFontOffsetForIndex(ttf_file.str, 0));

        for (u32 j = 0; j < faces[i].num_fonts; j++) {
            f32 scale = stbtt_ScaleForPixelHeight(&info, faces[i].fonts[j].size);

            i32 a, d, l;
            stbtt_GetFontVMetrics(&info, &a, &d, &l);

            i32 w, b;
            stbtt_GetCodepointHMetrics(&info, ' ', &w, &b);

            faces[i].fonts[j].ascent = (f32)(a * scale);
            faces[i].fonts[j].descent = (f32)(d * scale);
            faces[i].fonts[j].line_gap = (f32)(l * scale);
            faces[i].fonts[j].tab_size = (f32)(w * scale);
        }
    }

    stbtt_PackEnd(&pc);

    image img = {
        .valid = true,
        .channels = 1,
        .width = width,
        .height = height,
        .data = bitmap
    };

    font_img_id = draw_rectb_add_tex(rectb, img);
    draw_rectb_finalize_textures(rectb);

    marena_scratch_release(scratch);
}

void text_default(marena* arena, app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;

    *text = (pres_text){ 
        .col = (vec4d){ 1, 1, 1, 1 }
    };
}
void text_init(marena* arena, app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;

    i64 face = -1;
    for (u32 i = 0; i < num_faces; i++) {
        if (str8_equals(faces[i].name, text->font_name)) {
            face = i;
            break;
        }
    }
    if (face == -1) {
        face = default_font.face;
    }

    i64 font = -1;
    for (u32 i = 0; i < faces[face].num_fonts; i++) {
        if (faces[face].fonts[i].size == text->font_size) {
            font = i;
            break;
        }
    }
    if (font == -1) {
        font = default_font.font;
    }

    text->font.face = (u32)face;
    text->font.font = (u32)font;
}
void text_draw(app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;
    font_font* font = &faces[text->font.face].fonts[text->font.font];

    f32 start_x = text->x;
    f32 x = text->x;

    f32 line_height = font->ascent - font->descent + font->line_gap;
    f32 y = text->y + line_height;

    for (u64 i = 0; i < text->text.size; i++ ){
        u8 c = text->text.str[i];

        switch (c) {
            case '\n': {
                y += line_height;
                x = start_x;
            } break;
            case '\t': {
                x += font->tab_size;
            } break;
            default: break;
        }

        if (c < 32 || c > 127)
            continue;

        stbtt_packedchar* pc = &font->glyph_metrics[c - FIRST_CHAR];

        vec2 dim = { pc->x1 - pc->x0, pc->y1 - pc->y0 };

        draw_rectb_push_ex(
            rectb,
            (rect){ x + pc->xoff, y + pc->yoff, dim.x, dim.y },
            text->col,
            font_img_id,
            (rect){ pc->x0, pc->y0, dim.x, dim.y }
        );

        x += pc->xadvance;
    }

    draw_rectb_flush(rectb);
}

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    obj_desc font_desc = {
        .name = STR("font"),
        .obj_size = sizeof(pres_font),

        .desc_init_func = font_desc_init,
        .desc_destroy_func = font_desc_destroy,

        .default_func = font_default,
        .init_func = font_init,
        .file_func = font_file,

        .fields = {
            { STR("source" ), FIELD_STR8,    offsetof(pres_font, source    ) },
            { STR("sizes"  ), FIELD_F64_ARR, offsetof(pres_font, sizes     ) },
            { STR("default"), FIELD_BOOL32,  offsetof(pres_font, is_default) },
        }
    };

    obj_desc text_desc = {
        .name = STR("text"),
        .obj_size = sizeof(pres_text),

        .default_func = text_default,
        .init_func = text_init,
        .draw_func = text_draw,

        .fields = {
            { STR("text"     ), FIELD_STR8,  offsetof(pres_text, text     ) },
            { STR("font"     ), FIELD_STR8,  offsetof(pres_text, font_name) },
            { STR("font_size"), FIELD_F64,   offsetof(pres_text, font_size) },
            { STR("x"        ), FIELD_F64,   offsetof(pres_text, x        ) },
            { STR("y"        ), FIELD_F64,   offsetof(pres_text, y        ) },
            { STR("col"      ), FIELD_VEC4D, offsetof(pres_text, col      ) },
        }
    };

    obj_reg_add_desc(arena, app, app->pres->obj_reg, &font_desc);
    obj_reg_add_desc(arena, app, app->pres->obj_reg, &text_desc);
}
