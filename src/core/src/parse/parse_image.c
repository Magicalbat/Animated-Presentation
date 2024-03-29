#include "parse/parse.h"

static const string8 png_file_header = { (u8*)("\x89" "PNG" "\r\n" "\x1A" "\n"), 8 };
static const string8 qoi_file_header = { (u8*)("qoif"), 4 };

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

    marena_temp temp_arena;

    image png;

    u8* out;
    u64 out_size;
    u64 out_pos;
} pstate;

typedef struct {
    u8* data;
    u64 size;
} u8arr;

//static inline u8 ppeek_byte(pstate* state) {
//    return state->data[state->pos];
//}

static inline u8 pget_byte(pstate* state) {
    return state->data[state->pos++];
}

//#define PBYTE() (ppeek_byte(state))
#define BYTE()  (pget_byte(state))
#define U32()   (BYTE() << 24 | BYTE() << 16 | BYTE() << 8 | BYTE())

#define PNG_CHUNK_ID(a, b, c, d) ((u32)(a) << 24 | (u32)(b) << 16 | (u32)(c) << 8 | (u32)(d))

static void parse_png_ihdr(marena* arena, pstate* state, u32 num_channels) {
    state->png.width = U32();
    state->png.height = U32();

    state->bit_depth = BYTE();
    state->color_type = BYTE();

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

    state->out = CREATE_ZERO_ARRAY(arena, u8, state->png.width * state->png.height * num_channels);
    state->temp_arena = marena_temp_begin(arena);
}

static u8arr png_decompress(marena* arena, pstate* state) {
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

#define DF_CORE_SWITCH(for_header, A, B, C, index) \
    switch(filter_type) { \
        case PNG_NONE: \
            for_header { \
                state->out[state->out_pos] = data.data[index]; \
                state->out_pos++; \
            } \
            break; \
        case PNG_SUB: \
            for_header { \
                state->out[state->out_pos] = data.data[index] + A; \
                state->out_pos++; \
            } \
            break; \
        case PNG_UP: \
            for_header { \
                state->out[state->out_pos] = data.data[index] + B; \
                state->out_pos++; \
            } \
            break; \
        case PNG_AVG: \
            for_header { \
                state->out[state->out_pos] = data.data[index] + ( \
                    (A + B) >> 1); \
                state->out_pos++; \
            } \
            break; \
        case PNG_PAETH: \
            for_header { \
                state->out[state->out_pos] = data.data[index] + \
                    paeth_predictor(A, B, C); \
                state->out_pos++; \
            } \
            break; \
        default: \
            log_errorf("Invalid filter type %u, expected 0 - 4", filter_type); \
            return; \
            break; \
    } \

#define PIXEL_BYTES() bytes_per_pixel[state->color_type]

static void png_defilter(pstate* state, u8arr data, u32 num_channels) {
    pfilter filter_type = data.data[0];
    u64 byte_width = 1 + state->png.width * bytes_per_pixel[state->color_type];
    u64 real_byte_width = state->png.width * num_channels;

    if (bytes_per_pixel[state->color_type] >= 3 && bytes_per_pixel[state->color_type] == num_channels) {
        DF_CORE_SWITCH(
            for (u32 j = 1; j < PIXEL_BYTES() + 1; j++),
            0, 0, 0, j
        );
    
        DF_CORE_SWITCH(
            for (u32 j = PIXEL_BYTES() + 1; j < byte_width; j++),
            state->out[state->out_pos - PIXEL_BYTES()],
            0, 0, j
        );
    
        for (u32 i = 1; i < state->png.height; i++) {
            filter_type = data.data[i * byte_width];
    
            DF_CORE_SWITCH(
                for (u32 j = 1; j < PIXEL_BYTES() + 1; j++),
                0, state->out[state->out_pos - byte_width + 1], 0,
                j + i * byte_width
            );
    
            DF_CORE_SWITCH(
                for (u32 j = PIXEL_BYTES() + 1; j < byte_width; j++),
                state->out[state->out_pos - PIXEL_BYTES()],
                state->out[state->out_pos - byte_width + 1],
                state->out[state->out_pos - byte_width + 1 - PIXEL_BYTES()],
                j + i * byte_width
            );
        }
    } else if (bytes_per_pixel[state->color_type] == 3 && num_channels == 4) {
        DF_CORE_SWITCH(
            for (u32 j = 1; j < PIXEL_BYTES() + 1; j++),
            0, 0, 0, j
        );
        state->out[state->out_pos++] = 255;

        u64 third_width = (byte_width - 1) / 3;
        
        DF_CORE_SWITCH(
            for (u32 k = 1; k < third_width; k++, state->out[state->out_pos++] = 255)
                for (u32 j = k * 3 + 1; j < k * 3 + 1 + PIXEL_BYTES(); j++),
            state->out[state->out_pos - num_channels],
            0, 0, j
        );

        for (u32 i = 1; i < state->png.height; i++) {
            filter_type = data.data[i * byte_width];
    
            DF_CORE_SWITCH(
                for (u32 j = 1; j < PIXEL_BYTES() + 1; j++),
                0, state->out[state->out_pos - real_byte_width], 0,
                j + i * byte_width
            );
            state->out[state->out_pos++] = 255;
    
            DF_CORE_SWITCH(
                for (u32 k = 1; k < third_width; k++, state->out[state->out_pos++] = 255)
                    for (u32 j = k * 3 + 1; j < k * 3 + 1 + PIXEL_BYTES(); j++),
                state->out[state->out_pos - num_channels],
                state->out[state->out_pos - real_byte_width],
                state->out[state->out_pos - real_byte_width - num_channels],
                j + i * byte_width
            );
        }
    } else {
        log_error("Unsupported color type");
    }
}

static void parse_png_chunk(marena* arena, pstate* state, u32 num_channels) {
    state->chunk_size = U32();
    u32 chunk_id = U32();
    switch (chunk_id) {
        case PNG_CHUNK_ID('I', 'H', 'D', 'R'):
            state->chunk = PNG_IHDR;
            
            parse_png_ihdr(arena, state, num_channels);
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
            png_defilter(state, png_data, num_channels);
            
            state->pos += state->chunk_size + 4;
            break;
        default:
            //log_warnf("Unhandled PNG chunk %.*s", 4, state->data + state->pos - 4);
            state->pos += state->chunk_size + 4;
            break;
    }
}

static image parse_png(marena* arena, string8 file, u32 num_channels) {
    if (!str8_equals(png_file_header, str8_substr(file, 0, 8))) {
        log_error("Invalid PNG header, not a PNG file");
        return (image){ .valid = false };
    }

    pstate state = {
        .data = file.str + 8,
        .chunk = PNG_NULL,
        .png = (image){ .valid = true }
    };

    while (state.chunk != PNG_IEND) {
        parse_png_chunk(arena, &state, num_channels);
    }

    if (state.temp_arena.pos)
        marena_temp_end(state.temp_arena);

    state.png.channels = num_channels;
    state.png.data = state.out;
    return state.png;
}


#undef PBYTE
#undef BYTE
#undef U32

#define PBYTE() (data[pos])
#define BYTE()  (data[pos++])
#define U32() \
    (data[pos] << 24 | data[pos+1] << 16 | data[pos+2] << 8 | data[pos+3]); \
    pos += 4
#define PIXEL_INDEX(p) ((p.r * 3 + p.g * 5 + p.b * 7 + p.a * 11) & 63)
#define WRITE_PIXEL(p) do {       \
    out.data[out_pos + 0] = p.r; \
    out.data[out_pos + 1] = p.g; \
    out.data[out_pos + 2] = p.b; \
    out.data[out_pos + 3] = p.a; \
    out_pos += out.channels;     \
} while (0)

typedef struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} qpixel;

static image parse_qoi(marena* arena, string8 file, u32 num_channels) {
    if (file.size < 14 ||
        !str8_equals(qoi_file_header, str8_substr(file, 0, 4))) {
        log_error("Invalid QOI header. Not a QOI file");
        return (image){ .valid = false };
    }

    image out = { .valid = true };
    u8* data = file.str;
    u64 pos = 4;

    out.width = U32();
    out.height = U32();
    BYTE();
    out.channels = num_channels;
    u32 colorspace = BYTE();
    (void)colorspace;

    u64 out_pos = 0;
    u64 out_size = out.width * out.height * out.channels;
    // "+ 1" to prevent overflow in WRITE_PIXEL
    out.data = CREATE_ZERO_ARRAY(arena, u8, out_size + 1);

    qpixel arr[64] = { 0 };
    qpixel pixel = { .a = 255 };

    while (out_pos < out_size) {
        switch (PBYTE()) {
            case 0b11111110:
                pos++;
                pixel.r = BYTE();
                pixel.g = BYTE();
                pixel.b = BYTE();

                arr[PIXEL_INDEX(pixel)] = pixel;
                WRITE_PIXEL(pixel);

                break;
            case 0b11111111:
                pos++;
                pixel.r = BYTE();
                pixel.g = BYTE();
                pixel.b = BYTE();
                pixel.a = BYTE();

                arr[PIXEL_INDEX(pixel)] = pixel;
                WRITE_PIXEL(pixel);

                break;
            default:
                switch ((PBYTE() & 0b11000000) >> 6) {
                    case 0b00: ;
                        u8 index = BYTE() & 0b00111111;
                        pixel = arr[index];
                        WRITE_PIXEL(pixel);

                        break;
                    case 0b01: ;
                        u8 dr = ((PBYTE() & 0b00110000) >> 4) - 2;
                        u8 dg = ((PBYTE() & 0b00001100) >> 2) - 2;
                        u8 db = (( BYTE() & 0b00000011) >> 0) - 2;

                        pixel.r += dr;
                        pixel.g += dg;
                        pixel.b += db;

                        arr[PIXEL_INDEX(pixel)] = pixel;
                        WRITE_PIXEL(pixel);

                        break;
                    case 0b10: ;
                        u8 diff_g = (BYTE() & 0b00111111) - 32;
                        u8 diff_r = (((PBYTE() & 0b11110000) >> 4) - 8) + diff_g;
                        u8 diff_b = ((BYTE() & 0b00001111) - 8) + diff_g;

                        pixel.r += diff_r;
                        pixel.g += diff_g;
                        pixel.b += diff_b;

                        arr[PIXEL_INDEX(pixel)] = pixel;
                        WRITE_PIXEL(pixel);
 
                        break;
                    case 0b11: ;
                        u8 length = BYTE() & 0b00111111;

                        do {
                            WRITE_PIXEL(pixel);
                        } while (length--);

                        break;
                }
                break;
        }
    }

    marena_pop(arena, 4 - num_channels);
    return out;
}

image parse_image(marena* arena, string8 file, u32 num_channels) {
    if (str8_equals(png_file_header, str8_substr(file, 0, png_file_header.size))) {
        return parse_png(arena, file, num_channels);
    } else if (str8_equals(qoi_file_header, str8_substr(file, 0, qoi_file_header.size))) {
        return parse_qoi(arena, file, num_channels);
    }
    
    log_error("Unsupported image format");
    return (image){ 0 };
}
