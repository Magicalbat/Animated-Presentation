#ifndef PARSE_BITSTREAM_H
#define PARSE_BITSTREAM_H

#include "base/base_defs.h"

typedef struct {
    u8* data;
    u64 bit_pos;
    u64 num_bytes;
} bitstream_t;

u32 bs_peek_bits(bitstream_t* bs, u32 bits);
u32 bs_get_bits(bitstream_t* bs, u32 bits);
u8* bs_get_ptr(bitstream_t* bs);

#endif // PARSE_BITSTREAM_H