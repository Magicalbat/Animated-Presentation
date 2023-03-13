#include "app/app_pres.h"

#include <ctype.h>

/*
plugins = [
    "builtin_plugin"
]
slides = [
    slide {
        rectangle {
            x = 50,
            y = 50,
            w = 50, h = 50,
        }
    }
]
*/

typedef struct {
    string8 file;
    u64 pos;
} pres_parser;

static void parse_plugins(marena* arena, marena_temp scratch, apres* pres, pres_parser* parser);

static string8 parse_keyword(pres_parser* parser);
static string8 parse_string(marena* arena, pres_parser* parser);

#define P_CHAR(p) ((p)->file.str[(p)->pos])
#define P_NEXT_CHAR(p) do { \
        if ((p)->pos + 1 >= (p)->file.size) \
            log_error("Failed to parse presentation, past end of file"); \
        (p)->pos++; \
    } while (0)
#define P_WHITESPACE(p) ((p)->file.str[(p)->pos] == ' '  || (p)->file.str[(p)->pos] == '\t' || (p)->file.str[(p)->pos] == '\n' || (p)->file.str[(p)->pos] == '\r')
#define P_SKIP_SPACE(p) while (P_WHITESPACE((p))) { P_NEXT_CHAR((p)); }
#define P_KEY_CHAR(p) (isalnum((p)->file.str[(p)->pos]) || (p)->file.str[(p)->pos] == '_')

apres* pres_parse(marena* arena, ap_app* app, string8 file_path) {
    apres* pres = CREATE_ZERO_STRUCT(arena, apres);
    pres->obj_reg = obj_reg_create(arena, PRES_MAX_DESCS);
    
    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 file = os_file_read(scratch.arena, file_path);
    
    pres_parser parser = (pres_parser){
        .file = file,
        .pos = 0
    };
    
    b32 done = false;
    while (!done) {
        string8 keyword = parse_keyword(&parser);

        P_SKIP_SPACE(&parser);

        if (P_CHAR(&parser) != '=') {
            log_errorf("Invalid char '%c' after keyword, expected '='", P_CHAR(&parser));

            //marena_pop(arena, sizeof(apres));
            //return NULL;
            marena_scratch_release(scratch);
            return pres;
        }

        P_NEXT_CHAR(&parser);
        P_SKIP_SPACE(&parser);

        if (str8_equals(keyword, STR8_LIT("plugins"))) {
            parse_plugins(arena, scratch, pres, &parser);
        } else if (str8_equals(keyword, STR8_LIT("slides"))) {
            // TODO
        }

        while (!P_KEY_CHAR(&parser)) {
            if (parser.pos + 1 >= parser.file.size) {
                done = false;
                break;
            }
            parser.pos++;
        }
    }
    
    marena_scratch_release(scratch);

    return pres;
}

typedef void (plugin_init_func)(marena* arena, obj_register* obj_reg);
static void parse_plugins(marena* arena, marena_temp scratch, apres* pres, pres_parser* parser) {
    if (P_CHAR(parser) != '[') {
        log_errorf("Invalid char '%c' at plugins, expected '['", P_CHAR(parser));

        return;
    }

    P_NEXT_CHAR(parser);
    P_SKIP_SPACE(parser);

    marena_temp temp = marena_temp_begin(scratch.arena);

    string8_list plugin_names = { 0 };

    while (P_CHAR(parser) != ']') {
        string8 str = parse_string(temp.arena, parser);
        str8_list_push(temp.arena, &plugin_names, str);

        if (P_CHAR(parser) == ',') { P_NEXT_CHAR(parser); }

        P_SKIP_SPACE(parser);
    }

    pres->num_plugins = (u32)plugin_names.node_count;
    pres->plugins = CREATE_ZERO_ARRAY(arena, os_library, pres->num_plugins);

    string8_node* node = plugin_names.first;
    for (u32 i = 0; i < pres->num_plugins; i++) {
        pres->plugins[i] = os_lib_load(node->str);
        plugin_init_func* init_func = (plugin_init_func*)os_lib_func(pres->plugins[i], "plugin_init");
        init_func(arena, pres->obj_reg);

        node = node->next;
    }

    marena_temp_end(temp);
}

static string8 parse_keyword(pres_parser* parser) {
    P_SKIP_SPACE(parser);

    if (!P_KEY_CHAR(parser)) {
        log_errorf("Invalid keyword char '%c'", parser->file.str[parser->pos]);
        
        return (string8){ 0 };
    }
    
    string8 out = (string8) { .str = parser->file.str + parser->pos };
    
    while (P_KEY_CHAR(parser)) {
        out.size++;
        P_NEXT_CHAR(parser);
    }

    return out;
}
static string8 parse_string(marena* arena, pres_parser* parser) {
    P_SKIP_SPACE(parser);

    if (P_CHAR(parser) != '"') {
        log_errorf("Invalid char '%c', expected '\"'", P_CHAR(parser));

        return (string8){ 0 };
    }
    P_NEXT_CHAR(parser);

    u64 start_pos = parser->pos;
    u64 str_size = 0;
    
    b32 escaped = false;

    while (true) {
        if (!escaped && P_CHAR(parser) == '\\') {
            escaped = true;
            
            P_NEXT_CHAR(parser);
            continue;
        }

        if (P_CHAR(parser) == '\"' && !escaped)
            break;
        
        escaped = false;
        str_size++;
        P_NEXT_CHAR(parser);
    }

    parser->pos = start_pos;

    string8 out = {
        .size = str_size,
        .str = CREATE_ARRAY(arena, u8, str_size)
    };

    u64 pos = 0;
    for (u64 i = 0; i < out.size; i++) {
        if (P_CHAR(parser) == '\\') {
            P_NEXT_CHAR(parser);

            char c = ' ';
            switch (P_CHAR(parser)) {
                case '\"': c = '\"'; break;
                case '\\': c = '\\'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                default: {
                    log_errorf("Invalid escape char '%c'", P_CHAR(parser));
                    
                    return out;
                } break;
            }

            out.str[pos++] = c;
        } else {
            out.str[pos++] = P_CHAR(parser);
        }
        P_NEXT_CHAR(parser);
    }

    P_NEXT_CHAR(parser);
    
    return out;
}

void pres_delete(apres* pres) {
    for (u32 i = 0; i < pres->num_plugins; i++) {
        os_lib_release(pres->plugins[i]);
    }

    obj_reg_destroy(pres->obj_reg);
}

void pres_draw(apres* pres, ap_app* app) { }
void pres_update(apres* pres, f32 delta) { }
