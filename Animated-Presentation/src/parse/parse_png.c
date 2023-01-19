#include "parse/parse.h"

typedef enum {
    PNG_NULL,
    PNG_IHDR,
    PNG_IDAT,
    PNG_PLTE,
    PNG_IEND
} pchunk;

typedef enum {
    PNG_GRAY    = 0,
    PNG_COLOR   = 2,
    PNG_INDEX   = 3,
    PNG_GRAY_A  = 4,
    PNG_COLOR_A = 6
} pcolor;

typedef enum {
    PNG_NONE  = 0,
    PNG_SUB   = 1,
    PNG_UP    = 2,
    PNG_AVG   = 3,
    PNG_PAETH = 4
} pfilter;

static u32 bytes_per_pixel[] = {
    1, 0, 3, 3, 2, 0, 4
};

typedef struct idat_node {
    u8* data;
    u32 size;
    struct idat_node* next;
} idat_node;

typedef struct {
    u8* data;
    u64 pos;
    pchunk chunk;
    u32 chunk_size;

    u32 bit_depth;
    pcolor color_type;

    idat_node* idat_first; 
    idat_node* idat_last; 
    u64 idat_total_size;

    arena_temp temp_arena;

    image png;

    u8* out;
    u64 out_size;
    u64 out_pos;
} pstate;

typedef struct {
    u8* data;
    u64 size;
} u8arr;

static inline u8 ppeek_byte(pstate* state) {
    return state->data[state->pos];
}

static inline u8 pget_byte(pstate* state) {
    return state->data[state->pos++];
}

#define PBYTE() (ppeek_byte(state))
#define BYTE()  (pget_byte(state))
#define U32()   (BYTE() << 24 | BYTE() << 16 | BYTE() << 8 | BYTE())

static const char* file_header = "\x89" "PNG" "\r\n" "\x1A" "\n";

#define PNG_CHUNK_ID(a, b, c, d) ((u32)(a) << 24 | (u32)(b) << 16 | (u32)(c) << 8 | (u32)(d))

static void parse_png_ihdr(arena* arena, pstate* state) {
    state->png.width = U32();
    state->png.height = U32();

    state->bit_depth = BYTE();
    state->color_type = BYTE();
    state->png.channels = bytes_per_pixel[state->color_type];

    u32 compression_method = BYTE();
    u32 filter_method = BYTE();
    u32 interlace_method = BYTE();

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

    state->out = CREATE_ZERO_ARRAY(arena, state->out, u8, state->png.width * state->png.height * bytes_per_pixel[state->color_type]);
    state->temp_arena = arena_temp_begin(arena);
}

static u8arr png_decompress(arena* arena, pstate* state) {
    u8arr out = {
        .size = state->png.width * state->png.height * bytes_per_pixel[state->color_type] + state->png.height
    };
    out.data = CREATE_ARRAY(arena, u8, out.size),
    memset(out.data, 0, out.size);

    bitstream bs = {
        .data = CREATE_ARRAY(arena, u8, state->idat_total_size),
        .bit_pos = 16,
        .num_bytes = state->idat_total_size - 1
    };
    u64 pos = 0;
    for (idat_node* node = state->idat_first; node != NULL; node = node->next) {
        memcpy(bs.data + pos, node->data, node->size);
        pos += node->size;
    }

    parse_deflate(&bs, out.data, out.size);

    return out;
}
// https://www.w3.org/TR/png/#9Filter-type-4-Paeth
static i32 paeth_predictor(i32 a, i32 b, i32 c) {
    i32 p = a + b - c;
    i32 pa = abs(p-a);
    i32 pb = abs(p-b);
    i32 pc = abs(p-c);
    
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    
    return c;
}

#define DF_CORE_SWITCH(loop_start, loop_end, A, B, C, index) \
    switch(filter_type) { \
        case PNG_NONE: \
            loop_start \
                state->out[state->out_pos] = data.data[index]; \
                state->out_pos++; \
            loop_end \
            break; \
        case PNG_SUB: \
            loop_start \
                state->out[state->out_pos] = data.data[index] + A; \
                state->out_pos++; \
            loop_end \
            break; \
        case PNG_UP: \
            loop_start \
                state->out[state->out_pos] = data.data[index] + B; \
                state->out_pos++; \
            loop_end \
            break; \
        case PNG_AVG: \
            loop_start \
                state->out[state->out_pos] = data.data[index] + ( \
                    (A + B) / 2); \
                state->out_pos++; \
            loop_end \
            break; \
        case PNG_PAETH: \
            loop_start \
                state->out[state->out_pos] = data.data[index] + \
                    paeth_predictor(A, B, C); \
                state->out_pos++; \
            loop_end \
            break; \
        default: \
            log_errorf("Invalid filter type %u, expected 0 - 4", filter_type); \
            return; \
            break; \
    }

#define PIXEL_BYTES() bytes_per_pixel[state->color_type]

static void png_defilter(pstate* state, u8arr data) {
    pfilter filter_type = data.data[0];
    u64 byte_height = state->png.height;// * bytes_per_pixel[state->color_type];
    u64 byte_width = 1 + state->png.width * bytes_per_pixel[state->color_type];

    DF_CORE_SWITCH(
        for (u32 j = 1; j < 4; j++) {, },
        0, 0, 0, j
    );

    DF_CORE_SWITCH(
        for (u32 j = 4; j < byte_width; j++) {, },
        state->out[state->out_pos - PIXEL_BYTES()],
        0, 0, j
    );

    for (u32 i = 1; i < byte_height; i++) {
        filter_type = data.data[i * byte_width];

        DF_CORE_SWITCH(
            for (u32 j = 1; j < 4; j++) {, },
            0, state->out[state->out_pos - byte_width + 1], 0,
            j + i * byte_width
        );

        DF_CORE_SWITCH(
            for (u32 j = 4; j < byte_width; j++) {, },
            state->out[state->out_pos - PIXEL_BYTES()],
            state->out[state->out_pos - byte_width + 1],
            state->out[state->out_pos - byte_width + 1 - PIXEL_BYTES()],
            j + i * byte_width
        );
    }
}

static void parse_png_chunk(arena* arena, pstate* state) {
    state->chunk_size = U32();
    u32 chunk_id = U32();
    switch (chunk_id) {
        case PNG_CHUNK_ID('I', 'H', 'D', 'R'):
            state->chunk = PNG_IHDR;
            
            parse_png_ihdr(arena, state);
            state->pos += 4;
            
            break;
        case PNG_CHUNK_ID('I', 'D', 'A', 'T'):
            state->chunk = PNG_IDAT;
            
            idat_node* node = CREATE_STRUCT(arena, idat_node);
            *node = (idat_node){
                .data = state->data + state->pos,
                .size = state->chunk_size
            };
            SLL_PUSH_BACK(state->idat_first, state->idat_last, node);
            
            state->idat_total_size += state->chunk_size;
            state->pos += state->chunk_size + 4;
            
            break;
        case PNG_CHUNK_ID('P', 'L', 'T', 'E'):
            log_error("TODO: PLTE");
            state->chunk = PNG_PLTE;
            state->pos += state->chunk_size + 4;
            break;
        case PNG_CHUNK_ID('I', 'E', 'N', 'D'):
            state->chunk = PNG_IEND;

            u8arr png_data = png_decompress(arena, state);
            png_defilter(state, png_data);
            
            state->pos += state->chunk_size + 4;
            break;
        default:
            //log_warnf("Unhandled PNG chunk %.*s", 4, state->data + state->pos - 4);
            state->pos += state->chunk_size + 4;
            break;
    }
}

image parse_png(arena* arena, string8 file) {
    if (!str8_equals(str8_from_cstr((u8*)file_header), str8_substr(file, 0, 8))) {
        log_error("Invalid PNG header, not a PNG file");
        return (image){ .valid = true };
    }

    pstate state = {
        .data = file.str + 8,
        .chunk = PNG_NULL,
        .png = (image){ .valid = true }
    };

    while (state.chunk != PNG_IEND) {
        parse_png_chunk(arena, &state);
    }

    if (state.temp_arena.start_pos)
        arena_temp_end(state.temp_arena);
    
    state.png.data = state.out;
    return state.png;
}
