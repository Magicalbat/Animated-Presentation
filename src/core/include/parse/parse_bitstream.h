#ifndef PARSE_BITSTREAM_H
#define PARSE_BITSTREAM_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base_defs.h"

typedef struct {
    u8* data;
    u64 bit_pos;
    u64 num_bytes;
} bitstream;

AP_EXPORT u32 bs_peek_bits(bitstream* bs, u32 bits);
AP_EXPORT u32 bs_get_bits(bitstream* bs, u32 bits);
AP_EXPORT u8* bs_get_ptr(bitstream* bs);

#define BS_FLUSH_BYTE(bs) (      \
    ((bs)->bit_pos & 7) == 0 ? 0 : \
    0, (bs)->bit_pos += 8 - ((bs)->bit_pos & 7))
#define BS_I32(bs) ((u32)(bs_get_bits((bs), 16) | (bs_get_bits((bs), 16) << 16)))
#define BS_U32(bs) ((u32)(bs_get_bits((bs), 16) | (bs_get_bits((bs), 16) << 16)))

#ifdef __cplusplus
}
#endif
    
#endif // PARSE_BITSTREAM_H
