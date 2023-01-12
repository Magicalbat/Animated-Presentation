#include "parse/parse.h"

typedef enum {
    PNG_NULL,
    PNG_IHDR,
    PNG_IDAT,
    PNG_PLTE,
    PNG_IEND
} pchunk_t;

typedef enum {
    PNG_GRAY    = 0,
    PNG_COLOR   = 2,
    PNG_INDEX   = 3,
    PNG_GRAY_A  = 4,
    PNG_COLOR_A = 6
} pcolor_t;

typedef struct {
    u8* data;
    u64 pos;
    pchunk_t chunk;
    u32 chunk_size;

    u32 bit_depth;
    pcolor_t color_type;

    png_t png;
    u64 out_pos;
} pstate_t;

static inline u8 ppeek_byte(pstate_t* state) {
    return state->data[state->pos];
}

static inline u8 pget_byte(pstate_t* state) {
    return state->data[state->pos++];
}

#define PBYTE() (ppeek_byte(state))
#define BYTE()  (pget_byte(state))
#define U32() (BYTE() << 24 | BYTE() << 16 | BYTE() << 8 | BYTE())

static const string8_t file_header = STR8_LIT(
    "\x89" "PNG" "\r\n" "\x1A" "\n"
);

#define PNG_CHUNK_ID(a, b, c, d) ((u32)(a) << 24 | (u32)(b) << 16 | (u32)(c) << 8 | (u32)(d))

void parse_png_ihdr(arena_t* arena, pstate_t* state) {
    state->png.width = U32();
    state->png.height = U32();

    state->bit_depth = BYTE();
    state->color_type = BYTE();
    u32 compression_method = BYTE();
    u32 filter_method = BYTE();
    u32 interlace_method = BYTE();

    log_debugf("%u %u", state->bit_depth, state->color_type);
    
    if (compression_method != 0) {
        log_errorf("Invalid compression method %d", compression_method);
        state->png.valid = false;
        return;
    }
    if (filter_method != 0) {
        log_errorf("Invalid filter method %d", compression_method);
        state->png.valid = false;
        return;
    }
    if (interlace_method != 0) {
        log_error("Filtering is currently not supported");
        state->png.valid = false;
        return;
    }
}

void parse_png_chunk(arena_t* arena, pstate_t* state) {
    state->chunk_size = U32();
    log_debugf("%u", state->chunk_size);
    u32 chunk_id = U32();
    switch (chunk_id) {
        case PNG_CHUNK_ID('I', 'H', 'D', 'R'):
            log_info("ihdr");
            state->chunk = PNG_IHDR;
            parse_png_ihdr(arena, state);
            state->pos += 4;
            break;
        case PNG_CHUNK_ID('I', 'D', 'A', 'T'):
            log_error("TODO: IDAT");
            state->chunk = PNG_IDAT;
            state->pos += state->chunk_size + 4;
            break;
        case PNG_CHUNK_ID('P', 'L', 'T', 'E'):
            log_error("TODO: PLTE");
            state->chunk = PNG_PLTE;
            state->pos += state->chunk_size + 4;
            break;
        case PNG_CHUNK_ID('I', 'E', 'N', 'D'):
            log_error("TODO: IEND");
            state->chunk = PNG_IEND;
            state->pos += state->chunk_size + 4;
            break;
        default:
            log_warnf("Unhandled PNG chunk %.*s", 4, state->data + state->pos - 4);
            state->pos += state->chunk_size + 4;
            break;
    }
}

png_t parse_png(arena_t* arena, string8_t file) {
    if (!str8_equals(file_header, str8_substr(file, 0, 8))) {
        log_error("Invalid PNG header, not a PNG file");
        return (png_t){ .valid = true };
    }

    pstate_t state = {
        .data = file.str + 8,
        .chunk = PNG_NULL,
        .png = (png_t){ .valid = true }
    };

    while (state.chunk != PNG_IEND) {
        parse_png_chunk(arena, &state);
        //break;
    }

    return state.png;
}