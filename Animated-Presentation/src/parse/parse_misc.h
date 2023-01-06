#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#include "base/base.h"

typedef struct {
    u8* data;
    u64 size;
    b32 valid;
    string8_t name;
} gzip_t;

gzip_t parse_gzip(arena_t* arena, string8_t file);

void parse_deflate(bitstream_t* bs, u8* out, u64 out_size);

#endif // PARSE_MISC_H