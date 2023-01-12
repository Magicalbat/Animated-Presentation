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

#define BS_FLUSH_BYTE(bs) (      \
    (bs)->bit_pos & 7 == 0 ? 0 : \
    0, (bs)->bit_pos += 8 - ((bs)->bit_pos & 7))
#define BS_I32(bs) ((u32)(bs_get_bits((bs), 16) | (bs_get_bits((bs), 16) << 16)))
#define BS_U32(bs) ((u32)(bs_get_bits((bs), 16) | (bs_get_bits((bs), 16) << 16)))
    
#endif // PARSE_BITSTREAM_H
