#include "parse/parse_misc.h"

static const char* file_header = "qoif";

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
} qpixel_t;

image_t parse_qoi(arena_t* arena, string8_t file) {
    if (file.size < 14 ||
        !str8_equals(str8_from_cstr((u8*)file_header), str8_substr(file, 0, 4))) {
        log_error("Invalid QOI header. Not a QOI file");
        return (image_t){ .valid = false };
    }

    image_t out = { .valid = true };
    u8* data = file.str;
    u64 pos = 4;

    out.width = U32();
    out.height = U32();
    out.channels = BYTE();
    u32 colorspace = BYTE();

    u64 out_pos = 0;
    u64 out_size = out.width * out.height * out.channels;
    // "+ 1" to prevent overflow in WRITE_PIXEL
    out.data = CREATE_ZERO_ARRAY(arena, out.data, u8, out_size + 1);

    qpixel_t arr[64] = { 0 };
    qpixel_t pixel = { .a = 255 };

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
                    case 0b00:
                        u8 index = BYTE() & 0b00111111;
                        pixel = arr[index];
                        WRITE_PIXEL(pixel);

                        break;
                    case 0b01:
                        u8 dr = ((PBYTE() & 0b00110000) >> 4) - 2;
                        u8 dg = ((PBYTE() & 0b00001100) >> 2) - 2;
                        u8 db = (( BYTE() & 0b00000011) >> 0) - 2;

                        pixel.r += dr;
                        pixel.g += dg;
                        pixel.b += db;

                        arr[PIXEL_INDEX(pixel)] = pixel;
                        WRITE_PIXEL(pixel);

                        break;
                    case 0b10:
                        u8 diff_g = (BYTE() & 0b00111111) - 32;
                        u8 diff_r = (((PBYTE() & 0b11110000) >> 4) - 8) + diff_g;
                        u8 diff_b = ((BYTE() & 0b00001111) - 8) + diff_g;

                        pixel.r += diff_r;
                        pixel.g += diff_g;
                        pixel.b += diff_b;

                        arr[PIXEL_INDEX(pixel)] = pixel;
                        WRITE_PIXEL(pixel);
 
                        break;
                    case 0b11:
                        u8 length = BYTE() & 0b00111111;

                        do {
                            WRITE_PIXEL(pixel);
                        } while (length--);

                        break;
                }
                break;
        }
    }

    arena_pop(arena, 1);
    return out;
}
