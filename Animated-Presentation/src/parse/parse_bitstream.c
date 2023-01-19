#include "parse/parse_bitstream.h"

static u32 bit_masks[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

u32 bs_peek_bits(bitstream* bs, u32 bits) {
    u32 bytes = (bs->bit_pos >> 3) & ~1;
    u32 out = *(u32*)(bs->data + bytes);
    out >>= (bs->bit_pos & 0xf);
    return out & bit_masks[bits];
}
u32 bs_get_bits(bitstream* bs, u32 bits) {
    u32 bytes = (bs->bit_pos >> 3) & ~1;
    u32 out = *(u32*)(bs->data + bytes);
    out >>= (bs->bit_pos & 0xf);
    bs->bit_pos += bits;
    //log_debugf("bits %u = %u", bits, out & bit_masks[bits]);
    return out & bit_masks[bits];
}
u8* bs_get_ptr(bitstream* bs) {
    return bs->data + ((bs->bit_pos >> 3) & ~1); 
}
