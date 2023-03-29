#include "ap_core.h"
#include "draw/opengl_impl/gl_impl.h"
#include "app/app.h"

#include "stb_rect_pack.h"
#include "stb_truetype.h"

typedef struct {
    string8 source;
    f64 size;
    b32 is_default;

    u32 font_index;
} pres_font;

#define NUM_CHARS 95
#define START_CHAR 32

#define MAX_FACES 8
#define MAX_FONTS 8
static struct {
    stbtt_fontinfo font_info;
    f32 sizes[MAX_FONTS];
    stbtt_packedchar* glyph_metrics[MAX_FONTS];
    u32 num_sizes;

    string8 source;
    u64 reg_id;
    b32 loaded;
} faces[MAX_FACES] = { 0 };

typedef struct {
    u32 face_index;
    u32 font_index;
} font_ref;

typedef struct {
    rect draw_rect;
    vec4 col;
    rect tex_rect;
} text_draw_elem;

static font_ref default_font = { 0 };

void font_obj_default(marena* arena, app_app* app, void* obj) {
    pres_font* font = (pres_font*)obj;

    *font = (pres_font){
        .size = 12
    };
}

void font_obj_init(marena* arena, app_app* app, void* obj) {
    pres_font* font = (pres_font*)obj;
    i32 font_index = -1;
    for (i32 i = 0; i < MAX_FACES; i++) {
        if (str8_equals(font->source, faces[i].source)) {
            b32 size_found = false;
            for (i32 j = 0; j < MAX_FONTS; j++) {
                if (faces[i].sizes[j] == font->size) {
                    size_found = true;
                    break;
                }
            }

            if (!size_found) {
                for (i32 j = 0; j < MAX_FONTS; j++) {
                    if (faces[i].sizes[j] == 0) {
                        faces[i].sizes[j] = font->size;
                        faces[i].num_sizes++;
                        break;
                    }
                }
            }
            
            font_index = i;
            break;
        }
    }
    
    if (font_index == -1) {
        for (i32 i = 0; i < MAX_FACES; i++) {
            if (!faces[i].loaded) {
                font_index = i;

                faces[i].reg_id = str8_reg_push(app->temp.arena, &app->temp.file_reg, font->source);
                
                faces[i].sizes[0] = (f32)font->size;
                faces[i].num_sizes = 1;
                
                faces[i].source = font->source;
                faces[i].loaded = true;
                
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

// example: https://gist.github.com/vassvik/f442a4cc6127bc7967c583a12b148ac9

static b32 first_file_func = true;
void font_obj_file(marena* arena, app_app* app, void* obj) {
    if (!first_file_func) {
        return;
    }
    first_file_func = false;

    marena_temp scratch = marena_scratch_get(&arena, 1);
    
    u32 width = 1024;
    u32 height = 1024;
    u8* bitmap = CREATE_ZERO_ARRAY(app->temp.arena, u8, width * height);
    
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, bitmap, width, height, 0, 1, NULL);   
    stbtt_PackSetOversampling(&pc, 1, 1);
    
    for (u32 i = 0; i < MAX_FACES; i++) {
        if (faces[i].source.size == 0)
            continue;
        
        string8 file_str = str8_reg_get(&app->temp.file_reg, faces[i].reg_id);

        stbtt_pack_range* ranges = CREATE_ZERO_ARRAY(scratch.arena, stbtt_pack_range, faces[i].num_sizes);
        
        for (u32 j = 0; j < faces[i].num_sizes; j++) {
            faces[i].glyph_metrics[j] = CREATE_ZERO_ARRAY(arena, stbtt_packedchar, NUM_CHARS);
            ranges[j] = (stbtt_pack_range){
                .font_size = faces[i].sizes[j],
                .first_unicode_codepoint_in_range = START_CHAR,
                .num_chars = NUM_CHARS,
                .chardata_for_range = faces[i].glyph_metrics[j]
            };
        }

        int res = stbtt_PackFontRanges(&pc, file_str.str, 0, ranges, faces[i].num_sizes);
        if (res == 0) {
            log_error("Text plugin failed to pack all fonts");
            break;
        }
    }

    stbtt_PackEnd(&pc);

    // TODO: rendering

    marena_scratch_release(scratch);
}

typedef struct {
    f64 x, y;
    b32 center_x, center_y;
    string8 font_name;
    f64 font_size;
    string8 text;

    font_ref font;
} pres_text;

void text_obj_default(marena* arena, app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;

    *text = (pres_text) { };
}

void text_obj_init(marena* arena, app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;

    if (text->font_name.size == 0) {
        text->font = default_font;
    } else {
        for (u32 i = 0; i < MAX_FACES; i++ ){
            if (faces[i].num_sizes == 0)
                continue;
    
            string8 font_name = faces[i].source;
            
            u64 past_last_slash = 0;
            for (u64 i = 0; i < font_name.size - 1; i++) {
                if (font_name.str[i] == '/' || font_name.str[i] == '\\') {
                    past_last_slash = i + 1;
                }
            }
    
            font_name = str8_substr(font_name, past_last_slash, font_name.size);
    
            if (!str8_equals(font_name, text->font_name)) {
                continue;
            }
    
            for (u32 j = 0; j < MAX_FONTS; j++) {
                if (faces[i].sizes[j] != text->font_size) {
                    continue;
                }
    
                text->font = (font_ref) {
                    .face_index = i,
                    .font_index = j
                };
            }
        }
    }
}

void text_obj_draw(app_app* app, void* obj) {
    pres_text* text = (pres_text*)obj;
}

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    obj_desc font_desc = {
        .name = STR("font"),
        .obj_size = sizeof(pres_font),

        .default_func = font_obj_default,
        .init_func = font_obj_init,
        .file_func = font_obj_file,

        .fields = {
            { STR("source" ), FIELD_STR8,   offsetof(pres_font, source    ) },
            { STR("size"   ), FIELD_F64,    offsetof(pres_font, size      ) },
            { STR("default"), FIELD_BOOL32, offsetof(pres_font, is_default) },
        }
    };

    obj_desc text_desc = {
        .name = STR("text"),
        .obj_size = sizeof(pres_text),

        .default_func = text_obj_default,
        .init_func = text_obj_init,
        .draw_func = text_obj_draw,

        .fields = {
            { STR("x"        ), FIELD_F64,    offsetof(pres_text, x        ) },
            { STR("y"        ), FIELD_F64,    offsetof(pres_text, y        ) },
            { STR("center_x" ), FIELD_BOOL32, offsetof(pres_text, center_x ) },
            { STR("center_y" ), FIELD_BOOL32, offsetof(pres_text, center_y ) },
            { STR("font_name"), FIELD_STR8,   offsetof(pres_text, font_name) },
            { STR("font_size"), FIELD_F64,    offsetof(pres_text, font_size) },
            { STR("text"     ), FIELD_STR8,   offsetof(pres_text, text     ) },
        }
    };

    obj_reg_add_desc(app->pres->obj_reg, &font_desc);
    obj_reg_add_desc(app->pres->obj_reg, &text_desc);
}