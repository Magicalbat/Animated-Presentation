#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include "base/base.h"

typedef struct {
    b32 valid;
    u8* data;
    u64 size;
    string8_t name;
} gzip_t;

typedef struct {
    b32 valid;
    u8* data;
    u32 width;
    u32 height;
    u32 channels;
} png_t;

gzip_t parse_gzip(arena_t* arena, string8_t file);

// Always uses 8 bit depth
png_t parse_png(arena_t* arena, string8_t file);

void parse_deflate(bitstream_t* bs, u8* out, u64 out_size);

#endif // PARSE_MISC_H