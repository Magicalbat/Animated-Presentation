#include "app/app_pres.h"

#include <ctype.h>

typedef struct {
    string8 file;
    u64 pos;
    u64 line;
} pres_parser;

static void parse_next_char_noc(pres_parser* parser);
static void parse_next_char(pres_parser* parser);

static void parse_syntax_error(pres_parser* parser);

static void parse_settings(marena* arena, app_app* app, pres_parser* parser);
static void parse_plugins(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, pres_parser* parser);
static void parse_slides(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, pres_parser* parser);

static void parse_slide(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, app_slide_node* slide, pres_parser* parser);

static void parse_anim(marena* arena, marena_temp scratch, app_anim* anim, pres_parser* parser);

static field_val parse_field(marena* arena, marena_temp scratch, pres_parser* parser);
static field_val parse_arr(marena* arena, marena_temp sratch, pres_parser* parser);
static field_val parse_vec(pres_parser* parser);
static f64 parse_f64(pres_parser* parser);
static string8 parse_string(marena* arena, pres_parser* parser);
static string8 parse_keyword(pres_parser* parser);

#define P_CHAR(p) ((p)->file.str[(p)->pos])
#define P_WHITESPACE(p) ((p)->file.str[(p)->pos] == ' '  || (p)->file.str[(p)->pos] == '\t' || (p)->file.str[(p)->pos] == '\n' || (p)->file.str[(p)->pos] == '\r')
#define P_SKIP_SPACE(p) while (P_WHITESPACE((p))) { parse_next_char((p)); }
#define P_KEY_CHAR(p) (isalnum((p)->file.str[(p)->pos]) || (p)->file.str[(p)->pos] == '_')

app_pres* app_pres_parse(marena* arena, app_app* app, string8 file_path) {
    app_pres* pres = CREATE_ZERO_STRUCT(arena, app_pres);
    pres->obj_reg = obj_reg_create(arena, PRES_MAX_DESCS);

    app->pres = pres;
    
    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 file = os_file_read(scratch.arena, file_path);
    
    pres_parser parser = (pres_parser){
        .file = file,
        .line = 1
    };
    
    b32 done = false;
    while (!done) {
        string8 keyword = parse_keyword(&parser);

        P_SKIP_SPACE(&parser);

        if (P_CHAR(&parser) != '=') {
            log_errorf("Invalid char '%c' after keyword, expected '='", P_CHAR(&parser));
            parse_syntax_error(&parser);

            marena_pop(arena, sizeof(app_pres));
            return NULL;
        }

        parse_next_char(&parser);
        P_SKIP_SPACE(&parser);

        if (str8_equals(keyword, STR8_LIT("plugins"))) {
            parse_plugins(arena, scratch, app, pres, &parser);
        } else if (str8_equals(keyword, STR8_LIT("slides"))) {
            parse_slides(arena, scratch, app, pres, &parser);
        } else if (str8_equals(keyword, STR8_LIT("settings"))) {
            parse_settings(arena, app, &parser);
        } else {
            log_errorf("Invalid keyword \"%.*s\"", (int)keyword.size, keyword.str);
            parse_syntax_error(&parser);
        }

        while (!P_KEY_CHAR(&parser)) {
            if (parser.pos + 1 >= parser.file.size) {
                done = true;
                break;
            }

            if (parser.file.str[parser.pos] == '\n')
                parser.line++;

            parser.pos++;
        }
    }
    
    marena_scratch_release(scratch);

    return pres;
}

static void parse_next_char_noc(pres_parser* parser) {
    if (parser->pos + 1 >= parser->file.size)
        log_error("Failed to parse presentation, past end of file");
    parser->pos++;

    if (P_CHAR(parser) == '\n') {
        parser->line++;
    }
}
static void parse_next_char(pres_parser* parser) {
    if (parser->pos + 1 >= parser->file.size)
        log_error("Failed to parse presentation, past end of file");
    parser->pos++;

    if (P_CHAR(parser) == '\n') {
        parser->line++;
    }

    if (P_CHAR(parser) == '/') {
        if (parser->pos + 1 >= parser->file.size)
            return;

        if (parser->file.str[parser->pos+1] == '/') {
            parser->pos++;

            while (P_CHAR(parser) != '\n' && parser->pos < parser->file.size) {
                parser->pos++;
            }

            parser->pos++;
            parser->line++;
        } else if (parser->file.str[parser->pos+1] == '*') {
            parser->pos++;

            b32 asterisk = false;
            while (parser->pos < parser->file.size) {
                parser->pos++;

                if (asterisk && P_CHAR(parser) == '/')
                    break;

                asterisk = false;

                if (P_CHAR(parser) == '*')
                    asterisk = true;
                if (P_CHAR(parser) == '\n')
                    parser->line++;
            }
            parser->pos++;
        }
    }
}

static string8 get_line(string8 str, u64 pos) {
    u64 start_line = pos;
    while (start_line > 0 && str.str[start_line] != '\n')
        start_line--;
    if (start_line != 0) start_line++;

    u64 end_line = pos;
    while (end_line < str.size - 1 && str.str[end_line] != '\n')
        end_line++;

    return str8_substr(str, start_line, end_line);
}

static u32 num_digits (u64 n) {
    if (n < 10)                 return 1;
    if (n < 100)                return 2;
    if (n < 1000)               return 3;
    if (n < 10000)              return 4;
    if (n < 100000)             return 5;
    if (n < 1000000)            return 6;
    if (n < 10000000)           return 7;
    if (n < 100000000)          return 8;
    if (n < 1000000000)         return 9;
    if (n < 10000000000)        return 10;
    if (n < 100000000000)       return 11;
    if (n < 1000000000000)      return 12;
    if (n < 10000000000000)     return 13;
    if (n < 100000000000000)    return 14;
    if (n < 1000000000000000)   return 15;
    if (n < 10000000000000000)  return 16;
    if (n < 100000000000000000) return 17;
    return 18;
}

static u8 line_indicator[256];
static void parse_syntax_error(pres_parser* parser) {
    string8 line = get_line(parser->file, parser->pos);
    
    memset(line_indicator, ' ', sizeof(line_indicator));

    i64 line_pos = (i64)((parser->file.str + parser->pos) - line.str);
    u32 offset = num_digits(parser->line) + 1;
    
    for (i32 i = -2; i <= 2; i++) {
        line_indicator[MAX(0, line_pos + offset + i)] = '^';
    }
    string8 line_ind_str = (string8) { line_indicator, MIN(line.size + offset + 2, 256) };

    if (line.str - parser->file.str > 0) {
        string8 prev_line = get_line(
            parser->file,
            parser->pos - line_pos - 2
        );
        log_errorf("%llu: %.*s", parser->line - 1, (int)prev_line.size, prev_line.str);
    }

    log_errorf("%llu: %.*s", parser->line, (int)line.size, line.str);
    log_errorf("%.*s", (int)line_ind_str.size, line_ind_str.str);
}

#define SETTINGS_CASE(s) (str8_equals(keyword, STR8_LIT(s)))
static void parse_settings(marena* arena, app_app* app, pres_parser* parser) {
    if (P_CHAR(parser) != '[') {
        log_errorf("Invalid char '%c' at settings, expected '['", P_CHAR(parser));
        parse_syntax_error(parser);

        return;
    }

    parse_next_char(parser);
    P_SKIP_SPACE(parser);

    while (P_CHAR(parser) != ']') {
        string8 keyword = parse_keyword(parser);

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) != '=') {
            log_errorf("Invalid char '%c' after settings keyword, expected '='", P_CHAR(parser));
            parse_syntax_error(parser);

            break;
        }
        parse_next_char(parser);
        P_SKIP_SPACE(parser);

        if (SETTINGS_CASE("reference_dim")) {
            field_val dim = parse_vec(parser);
            if (dim.type != FIELD_VEC2D) {
                log_error("Invalid type for reference dim, expected VEC2D");
                parse_syntax_error(parser);

                break;
            }
            
            app->ref_width = (f32)dim.val.vec2d.x;
            app->ref_height = (f32)dim.val.vec2d.y;
        } else if (SETTINGS_CASE("background_color")) {
            field_val col = parse_vec(parser);
            if (col.type != FIELD_VEC4D) {
                log_error("Invalid type for reference dim, expected VEC4D");
                parse_syntax_error(parser);

                break;
            }

            app->bg_col = col.val.vec4d;
        } else if (SETTINGS_CASE("title")) {
            string8 title = parse_string(arena, parser);
            gfx_win_set_title(app->win, title);
        } else {
            log_errorf("Invalid keyword \"%.*s\" in settings", (int)keyword.size, keyword.str);
            parse_syntax_error(parser);

            break;
        }

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) == ',') { parse_next_char(parser); }
        P_SKIP_SPACE(parser);
    }
}

typedef void (plugin_init_func)(marena* arena, app_app* app);
static void parse_plugins(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, pres_parser* parser) {
    if (P_CHAR(parser) != '[') {
        log_errorf("Invalid char '%c' at plugins, expected '['", P_CHAR(parser));
        parse_syntax_error(parser);

        return;
    }

    parse_next_char(parser);
    P_SKIP_SPACE(parser);

    marena_temp temp = marena_temp_begin(scratch.arena);

    string8_list plugin_names = { 0 };

    string8 bin_path = os_binary_path();
    string8 plugin_str = STR8_LIT("/plugins/");
    string8 plugin_path = {
        .size = bin_path.size + plugin_str.size,
        .str = CREATE_ZERO_ARRAY(scratch.arena, u8, bin_path.size + plugin_str.size)
    };
    memcpy(plugin_path.str, bin_path.str, bin_path.size);
    memcpy(plugin_path.str + bin_path.size, plugin_str.str, plugin_str.size);

    string8 suffix = {};

#if defined(__linux__)
    suffix = STR8_LIT(".so");
#elif defined(_WIN32)
    suffix = STR8_LIT(".dll");
#elif defined(__EMSCRIPTEN__)
    suffix = STR8_LIT(".wasm");
#endif

    while (P_CHAR(parser) != ']') {
        string8 plugin_name = parse_string(temp.arena, parser);
        if (plugin_name.size == 0) {
            break;
        }

        u64 path_size = plugin_path.size + plugin_name.size + suffix.size;
        string8 path = {
            .size = path_size,
            .str = CREATE_ZERO_ARRAY(scratch.arena, u8, path_size)
        };
        memcpy(path.str, plugin_path.str, plugin_path.size);
        memcpy(path.str + plugin_path.size, plugin_name.str, plugin_name.size);
        memcpy(path.str + plugin_path.size + plugin_name.size, suffix.str, suffix.size);

        str8_list_push(temp.arena, &plugin_names, path);

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) == ',') { parse_next_char(parser); }
        P_SKIP_SPACE(parser);
    }

    pres->num_plugins = (u32)plugin_names.node_count;
    pres->plugins = CREATE_ZERO_ARRAY(arena, os_library, pres->num_plugins);

    string8_node* node = plugin_names.first;
    for (u32 i = 0; i < pres->num_plugins && node != NULL; i++) {
        pres->plugins[i] = os_lib_load(node->str);
        plugin_init_func* init_func = (plugin_init_func*)os_lib_func(pres->plugins[i], "plugin_init");
        init_func(arena, app);

        node = node->next;
    }

    marena_temp_end(temp);
}

static void parse_slides(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, pres_parser* parser) {
    if (P_CHAR(parser) != '[') {
        log_errorf("Invalid char '%c' at slides, expected '['", P_CHAR(parser));
        parse_syntax_error(parser);

        return;
    }
    
    parse_next_char(parser);
    P_SKIP_SPACE(parser);
    
    while (P_CHAR(parser) != ']') {
        string8 keyword = parse_keyword(parser);

        b32 is_global = str8_equals(keyword, STR8_LIT("global"));

        if (!is_global && !str8_equals(keyword, STR8_LIT("slide"))) {
            log_errorf("Invalid keyword \"%.*s\", expected \"slide\"", (int)keyword.size, keyword.str);
            parse_syntax_error(parser);

            break;
        }
        
        P_SKIP_SPACE(parser);

        app_slide_node* slide = CREATE_ZERO_STRUCT(arena, app_slide_node);
        slide->objs = app_objp_create(arena, pres->obj_reg, PRES_MAX_OBJS);
        slide->anims = app_animp_create(arena, PRES_MAX_ANIMS);
        
        if (is_global) {
            pres->global_slide = slide;
        } else {
            DLL_PUSH_BACK(pres->first_slide, pres->last_slide, slide);
            pres->num_slides++;
            pres->cur_slide = pres->first_slide;
        }

        parse_slide(arena, scratch, app, pres, slide, parser);

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) != '}') {
            log_errorf("Invalid char '%c' at end of slide, expected '}'", P_CHAR(parser));
            parse_syntax_error(parser);

            break;
        }

        parse_next_char(parser);
        P_SKIP_SPACE(parser);

        if (P_CHAR(parser) == ',') parse_next_char(parser);

        P_SKIP_SPACE(parser);
    }
}

static void parse_slide(marena* arena, marena_temp scratch, app_app* app, app_pres* pres, app_slide_node* slide, pres_parser* parser) {
    if (P_CHAR(parser) != '{') {
        log_errorf("Invalid char '%c' at slide, expected '{'", P_CHAR(parser));
        parse_syntax_error(parser);

        return;
    }

    parse_next_char(parser);
    P_SKIP_SPACE(parser);

    while (P_CHAR(parser) != '}') {
        if (!P_KEY_CHAR(parser)) {
            log_errorf("Invalid char '%c' for object type, expected letter, number, or underscore", P_CHAR(parser));
            parse_syntax_error(parser);
            break;
        }

        string8 obj_name = parse_keyword(parser);
        
        app_obj_ref ref = app_objp_add(slide->objs, pres->obj_reg, obj_name, arena, app);
        if (ref.obj == NULL) {
            log_errorf("Invalid object name \"%.*s\"", (int)obj_name.size, obj_name.str);
            parse_syntax_error(parser);

            break;
        }

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) != '{') {
            log_errorf("Invalid char '%c' at object, expected '{'", P_CHAR(parser));
            parse_syntax_error(parser);
            
            break;
        }
        parse_next_char(parser);
        P_SKIP_SPACE(parser);

        while (P_CHAR(parser) != '}') {
            if (!P_KEY_CHAR(parser)) {
                log_errorf("Invalid char '%c' for object field, expected letter, number, or underscore", P_CHAR(parser));
                parse_syntax_error(parser);
                break;
            }
            
            string8 field_name = parse_keyword(parser);
            
            P_SKIP_SPACE(parser);
            if (P_CHAR(parser) != '=') {
                log_errorf("Invalid char '%c' after object field, expected '='", P_CHAR(parser));
                parse_syntax_error(parser);
                break;
            }
            parse_next_char(parser);
            P_SKIP_SPACE(parser);

            if (P_CHAR(parser) == 'a') {
                app_anim* anim = app_animp_next(slide->anims);
                obj_desc* cur_desc = &pres->obj_reg->descs[ref.desc_index];
                
                i64 field_index = -1;
                for (u32 i = 0; i < OBJ_DESC_MAX_FIELDS; i++) {
                    if (str8_equals(cur_desc->fields[i].name, field_name))
                        field_index = i;
                }
                if (field_index == -1) {
                    log_errorf("Invalid field name \"%.*s\"", (int)field_name.size, field_name.str);
                }

                anim->obj_field = (void*)((u8*)ref.obj + cur_desc->fields[field_index].offset);

                parse_anim(arena, scratch, anim, parser);
                app_anim_finalize(arena, anim);
            } else {
                field_val field = parse_field(arena, scratch, parser);
                app_obj_ref_set(ref, pres->obj_reg, field_name, &field.val);
            }

            P_SKIP_SPACE(parser);
            if (P_CHAR(parser) == ',') { parse_next_char(parser); }
            P_SKIP_SPACE(parser);
        }

        app_obj_ref_init(ref, pres->obj_reg, arena, app);
        
        parse_next_char(parser);
        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) == ',') { parse_next_char(parser); }
        P_SKIP_SPACE(parser);
    }
}

static const u32 elem_types[FIELD_COUNT] = {
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,

    FIELD_F64,
    FIELD_STR8,
    FIELD_BOOL32,
    FIELD_VEC2D,
    FIELD_VEC3D,
    FIELD_VEC4D
};

#define ANIM_FIELD_CASE(name) (str8_equals(field_name, STR8_LIT(name)))
#define ANIM_TRY_NUM_KEYS(new_num) do { \
        if (anim->num_keys == 0) { \
            anim->num_keys = new_num; \
        } else if (anim->num_keys != new_num) { \
            log_error("Anim keys, times, and pauses all must be the same size"); \
            parse_syntax_error(parser); \
            return; \
        } \
    } while (0)
static void parse_anim(marena* arena, marena_temp scratch, app_anim* anim, pres_parser* parser) {
    string8 anim_keyword = parse_keyword(parser); 
    if (!str8_equals(anim_keyword, STR8_LIT("anim"))) {
        log_errorf("Invalid keyword \"%.*s\", expected \"anim\"", (int)anim_keyword.size, anim_keyword.str);
        parse_syntax_error(parser);

        return;
    }

    P_SKIP_SPACE(parser);

    if (P_CHAR(parser) != '{') {
        log_errorf("Invalid char '%c' at anim, expected '{'", P_CHAR(parser));
        parse_syntax_error(parser);

        return;
    }
    parse_next_char(parser);
    P_SKIP_SPACE(parser);

    while (P_CHAR(parser) != '}') {
        string8 field_name = parse_keyword(parser);

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) != '=') {
            log_errorf("Invalid char '%c' for anim field name, expected '='", P_CHAR(parser));
            parse_syntax_error(parser);
            return;
        }
        parse_next_char(parser);
        P_SKIP_SPACE(parser);

        if (ANIM_FIELD_CASE("time")) {
            anim->time = parse_f64(parser);
        } else if (ANIM_FIELD_CASE("repeat")) {
            string8 repeat_str = parse_keyword(parser);

            if (str8_equals(repeat_str, STR8_LIT("STOP"))) {
                anim->repeat = ANIM_STOP;
            } else if (str8_equals(repeat_str, STR8_LIT("LOOP"))) {
                anim->repeat = ANIM_LOOP;
            } else if (str8_equals(repeat_str, STR8_LIT("BOUNCE"))) {
                anim->repeat = ANIM_BOUNCE;
            }
        } else if (ANIM_FIELD_CASE("keys")) {
            field_val keys_field = parse_arr(arena, scratch, parser);
            anim->type = elem_types[keys_field.type];

            ANIM_TRY_NUM_KEYS(keys_field.val.f64_arr.size);
            anim->keys = (void*)keys_field.val.f64_arr.data;
        } else if (ANIM_FIELD_CASE("times")) {
            field_val times_field = parse_arr(arena, scratch, parser);
            if (times_field.type != FIELD_F64_ARR) {
                log_error("Anim times must be F64 array");
                parse_syntax_error(parser);

                return;
            }

            ANIM_TRY_NUM_KEYS(times_field.val.f64_arr.size);
            anim->times = times_field.val.f64_arr.data;
        } else if (ANIM_FIELD_CASE("pauses")) {
            field_val pauses_field = parse_arr(arena, scratch, parser);
            if (pauses_field.type != FIELD_BOOL32_ARR) {
                log_error("Anim times must be BOOL32 array");
                parse_syntax_error(parser);

                return;
            }

            ANIM_TRY_NUM_KEYS(pauses_field.val.bool32_arr.size);
            anim->pauses = pauses_field.val.bool32_arr.data;
        } else if (ANIM_FIELD_CASE("bezier")) {
            field_val bezier_field = parse_vec(parser);
            if (bezier_field.type != FIELD_VEC4D) {
                log_error("Bezier must be VEC4D");
                parse_syntax_error(parser);

                return;
            }
            
            vec4d v = bezier_field.val.vec4d;
            anim->bezier = (cbezier){
                .p = {
                    (vec2){ 0, 0 },
                    (vec2){ (f32)v.x, (f32)v.y },
                    (vec2){ (f32)v.z, (f32)v.w },
                    (vec2){ 1, 1 },
                }
            };
        } else {
            log_errorf("Invalid field name \"%.*s\" in anim", (int)field_name.size, field_name.str);
            parse_syntax_error(parser);

            return;
        }

        P_SKIP_SPACE(parser);
        if (P_CHAR(parser) == ',') { parse_next_char(parser); }
        P_SKIP_SPACE(parser);
    }

    parse_next_char(parser);
}

static field_val parse_field(marena* arena, marena_temp scratch, pres_parser* parser) {
    char c = P_CHAR(parser);
    field_val out = { 0 };

    if (isdigit(c) || c == '.' || c == '+' || c == '-') {
        out.type = FIELD_F64;
        out.val.f64 = parse_f64(parser);
    } else if (c == '\"') {
        out.type = FIELD_STR8;
        out.val.str8 = parse_string(arena, parser);
    } else if (c == 'v') {
        out = parse_vec(parser);
    } else if (c == 't' || c == 'f') {
        if (str8_equals(str8_substr_size(parser->file, parser->pos, 4), STR8_LIT("true"))) {
            out.type = FIELD_BOOL32;
            out.val.bool32 = true;
            
            parser->pos += 4;
        } else if (str8_equals(str8_substr_size(parser->file, parser->pos, 5), STR8_LIT("false"))) {
            out.type = FIELD_BOOL32;
            out.val.bool32 = false;
            
            parser->pos += 5;
        } else {
            log_error("Invalid value for boolean, expected true or false");
        }
    } else if (c == '[') {
        out = parse_arr(arena, scratch, parser);
    } else {
        log_errorf("Invalid char '%c' for field value", c);
        parse_syntax_error(parser);
    }

    return out;
}

typedef struct field_node {
    field_val field;
    struct field_node* next;
} field_node;

typedef struct {
    field_node* first;
    field_node* last;
    u64 size;
} field_list;

static const field_type arr_types[] = {
    FIELD_NULL,

    FIELD_F64_ARR,
    FIELD_STR8_ARR,
    FIELD_BOOL32_ARR,
    FIELD_VEC2D_ARR,
    FIELD_VEC3D_ARR,
    FIELD_VEC4D_ARR,

    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
    FIELD_NULL,
};
static const u32 elem_sizes[] = {
    0,

    sizeof(f64),
    sizeof(string8),
    sizeof(b32),
    sizeof(vec2d),
    sizeof(vec3d),
    sizeof(vec4d),
};
const char* field_names[FIELD_COUNT] = {
#define X(a) #a,
    FIELD_XLIST
#undef X
};

static field_val parse_arr(marena* arena, marena_temp scratch, pres_parser* parser) {
    if (P_CHAR(parser) != '[') {
        log_errorf("Invalid '%c' for list, expected '['", P_CHAR(parser));
        parse_syntax_error(parser);
        
        return (field_val){ 0 };
    }
    parse_next_char(parser);
    P_SKIP_SPACE(parser);

    marena_temp temp = marena_temp_begin(scratch.arena);
    field_list list = { 0 };

    field_val out = { 0 };
    field_type elem_type = FIELD_NULL;

    while (P_CHAR(parser) != ']') {
        field_val field = parse_field(arena, scratch, parser);
        if (P_CHAR(parser) == ',') { parse_next_char(parser); }
        
        if (elem_type == FIELD_NULL) {
            field_type type = arr_types[field.type];
            if (type == FIELD_NULL) {
                log_errorf("Invalid array type %s", field_names[type]);
                parse_syntax_error(parser);
                
                marena_temp_end(temp);
                return (field_val) { 0 };
            }
            elem_type = field.type;
            out.type = type;
        } else if (elem_type != field.type) {
            log_errorf("Invalid field of type %s in %s array", field_names[field.type], field_names[elem_type]);
            parse_syntax_error(parser);
            
            marena_temp_end(temp);
            return (field_val) { 0 };
        }

        field_node* node = CREATE_STRUCT(temp.arena, field_node);
        node->field = field;
        
        SLL_PUSH_BACK(list.first, list.last, node);
        list.size++;
        
        P_SKIP_SPACE(parser);
    }
    parse_next_char(parser);

    // All arrays have the same layout in memory,
    // so I am using f64 as a proxy
    out.val.f64_arr.size = list.size;
    out.val.f64_arr.data = (f64*)marena_push_zero(arena, elem_sizes[elem_type] * list.size);
    
    u64 i = 0;
    for (field_node* node = list.first; node != NULL; node = node->next) {
        memcpy((u8*)out.val.f64_arr.data + elem_sizes[elem_type] * i, &node->field.val, elem_sizes[elem_type]);
        i++;
    }

    marena_temp_end(temp);

    return out;
}

static const field_type vec_types[] = {
    FIELD_NULL,
    FIELD_NULL,
    FIELD_VEC2D,
    FIELD_VEC3D,
    FIELD_VEC4D,
};
static field_val parse_vec(pres_parser* parser) {
    string8 keyword = parse_keyword(parser);
    string8 test_str = str8_substr_size(keyword, 0, 3);
    if (!str8_equals(test_str, STR8_LIT("vec"))) {
        log_errorf("Invalid keyword \"%.*s\", expected vec", (int)test_str.size, test_str.str);
        parse_syntax_error(parser);
        
        return (field_val){ 0 };
    }
    P_SKIP_SPACE(parser);
    if (P_CHAR(parser) != '{') {
        log_errorf("Invalid char '%c' for vector, expected '{'", P_CHAR(parser));
        parse_syntax_error(parser);
        
        return (field_val){ 0 };
    }
    
    u32 vec_len = 0;
    switch (keyword.str[3]) {
        case '2': vec_len = 2; break;
        case '3': vec_len = 3; break;
        case '4': vec_len = 4; break;
        default: vec_len = 2; break;
    }

    field_val out = { .type = vec_types[vec_len] };
    
    if (out.type == FIELD_NULL) {
        log_error("Invalid type for vector");
        parse_syntax_error(parser);
    }

    for (u32 i = 0; i < vec_len; i++) {
        parse_next_char(parser);
        P_SKIP_SPACE(parser);
        out.val.vec4d.p[i] = parse_f64(parser);
    }
    P_SKIP_SPACE(parser);
    if (P_CHAR(parser) == ',') { parse_next_char(parser); }
    P_SKIP_SPACE(parser);
    
    if (P_CHAR(parser) != '}') {
        log_errorf("Invalid char '%c' after vector, expected '}'", P_CHAR(parser));
        parse_syntax_error(parser);
        
        return (field_val){ 0 };
    }
    parse_next_char(parser);

    return out;
}
static f64 parse_f64(pres_parser* parser) {
    char c = P_CHAR(parser);
    if (!isdigit(c) && c != '.' && c != '+' && c != '-') {
        log_errorf("Invalid char '%c' for float 64", c);
        parse_syntax_error(parser);
        return 0.0;
    }

    char* start_ptr = (char*)(parser->file.str + parser->pos);
    
    while (isdigit(c) || c == '.' || c == '+' || c == '-') {
        parse_next_char(parser);
        c = P_CHAR(parser);
    }

    char* unused_ptr = NULL;

    // Is this unsafe becuase it it not length based?
    // I will leave that as an exercize for the reader.
    f64 out = (f64)(strtod(start_ptr, &unused_ptr));

    return out;
}
static string8 parse_string(marena* arena, pres_parser* parser) {
    P_SKIP_SPACE(parser);

    if (P_CHAR(parser) != '"') {
        log_errorf("Invalid char '%c', expected '\"'", P_CHAR(parser));
        parse_syntax_error(parser);

        return (string8){ 0 };
    }
    parse_next_char(parser);

    u64 start_pos = parser->pos;
    u64 str_size = 0;
    
    b32 escaped = false;

    while (true) {
        if (!escaped && P_CHAR(parser) == '\\') {
            escaped = true;
            
            parse_next_char_noc(parser);
            continue;
        }

        if (P_CHAR(parser) == '\"' && !escaped)
            break;
        
        escaped = false;
        str_size++;
        parse_next_char_noc(parser);
    }

    string8 out = {
        .size = str_size,
        .str = CREATE_ARRAY(arena, u8, str_size)
    };

    u8* ptr = &parser->file.str[start_pos];
    u8* end = &parser->file.str[parser->pos];

    u64 pos = 0;
    for (;ptr < end; ptr += 1) {
        if (*ptr == '\\') {
            ptr += 1;

            char c = ' ';
            switch (*ptr) {
                case '\"': c = '\"'; break;
                case '\\': c = '\\'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                default: {
                    log_errorf("Invalid escape char '%c' in string", *ptr);
                    parse_syntax_error(parser);
                    
                    return out;
                } break;
            }

            out.str[pos++] = c;
        } else {
            out.str[pos++] = *ptr;
        }
    }

    parse_next_char(parser);
    
    return out;
}
static string8 parse_keyword(pres_parser* parser) {
    P_SKIP_SPACE(parser);

    if (!P_KEY_CHAR(parser)) {
        log_errorf("Invalid keyword char '%c'", parser->file.str[parser->pos]);
        parse_syntax_error(parser);
        
        return (string8){ 0 };
    }
    
    string8 out = (string8) { .str = parser->file.str + parser->pos };
    
    while (P_KEY_CHAR(parser)) {
        out.size++;
        parse_next_char(parser);
    }

    return out;
}

