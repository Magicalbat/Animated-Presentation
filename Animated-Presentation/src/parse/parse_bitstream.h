#ifndef PARSE_BITSTREAM_H
#define PARSE_BITSTREAM_H

#include "base/base_defs.h"

typedef struct {
    u8* data;
    u64 bit_pos;
    u64 num_bytes;
} bitstream_t;


extern u32 bit_masks[];

inline u32 bs_peek_bits(bitstream_t* bs, u32 bits) {
    u32 bytes = (bs->bit_pos >> 3) & ~1;
    u32 out = *(u32*)(bs->data + bytes);
    out >>= (bs->bit_pos & 0xf);
    return out & bit_masks[bits];
}
inline u32 bs_get_bits(bitstream_t* bs, u32 bits) {
    u32 bytes = (bs->bit_pos >> 3) & ~1;
    u32 out = *(u32*)(bs->data + bytes);
    out >>= (bs->bit_pos & 0xf);
    bs->bit_pos += bits;
    //log_debugf("bits %u = %u", bits, out & bit_masks[bits]);
    return out & bit_masks[bits];
}
inline u8* bs_get_ptr(bitstream_t* bs) {
    return bs->data + ((bs->bit_pos >> 3) & ~1); 
}

#endif // PARSE_BITSTREAM_H
