#include "app/app_pres.h"

#include <ctype.h>

/*
plugins = [
    "builtin_plugin"
],
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

static string8 pres_parse_keyword(pres_parser* parser);
static string8 pres_parse_string(marena_temp scratch, pres_parser* parser);

#define P_CHAR(p) ((p)->file.str[(p)->pos])
#define P_NEXT_CHAR(p) do { \
        (p)->pos++; \
        if ((p)->pos >= (p)->file.size) \
            log_error("Failed to parse presentation, out of file boundary"); \
    } while (0)
#define P_WHITESPACE(p) ((p)->file.str[(p)->pos] == ' '  || (p)->file.str[(p)->pos] == '\t' || (p)->file.str[(p)->pos] == '\n' || (p)->file.str[(p)->pos] == '\r')
#define P_SKIP_SPACE(p) while (P_WHITESPACE((p))) { P_NEXT_CHAR((p)); }
#define P_KEY_CHAR(p) (isalnum((p)->file.str[(p)->pos]) || (p)->file.str[(p)->pos] == '_')

apres* pres_parse(marena* arena, string8 file_path) {
    apres* out = CREATE_ZERO_STRUCT(arena, apres);
    
    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 file = os_file_read(scratch.arena, file_path);
    
    pres_parser parser = (pres_parser){
        .file = file,
        .pos = 0
    };
    
    string8 keyword = pres_parse_keyword(&parser);
    log_debugf("%.*s", keyword.size, keyword.str);
    
    marena_scratch_release(scratch);

    return out;
}

static string8 pres_parse_keyword(pres_parser* parser) {
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
static string8 pres_parse_string(marena_temp scratch, pres_parser* parser) {
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
        .str = CREATE_ARRAY(scratch.arena, u8, str_size)
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
    
    return out;
}

void slide_draw(slide_node* slide, ap_app* app) { }
void slide_update(slide_node* slide, f32 delta) { }