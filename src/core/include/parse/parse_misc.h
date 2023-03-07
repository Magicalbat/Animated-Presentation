#ifndef PARSE_MISC_H
#define PARSE_MISC_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "parse/parse_bitstream.h"

// ATTENTION!
// Many of these parsers are not compliant to specification
// These should not be used in real world applications

typedef struct {
    b32 valid;
    u8* data;
    u64 size;
    string8 name;
} gzip;

typedef struct {
    b32 valid;
    u8* data;
    u32 width;
    u32 height;
    u32 channels;
} image;

AP_EXPORT void parse_deflate(bitstream* bs, u8* out, u64 out_size);

AP_EXPORT gzip parse_gzip(marena* arena, string8 file);

AP_EXPORT image parse_image(marena* arena, string8 file, u32 num_channels);

#ifdef __cplusplus
}
#endif

#endif // PARSE_MISC_H
