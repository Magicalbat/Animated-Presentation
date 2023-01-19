#include "parse/parse.h"

#define FLG_TEXT    (1 << 0)
#define FLG_HCRC    (1 << 1)
#define FLG_EXTRA   (1 << 2)
#define FLG_NAME    (1 << 3)
#define FLG_COMMENT (1 << 4)

#define BITS(n) (bs_get_bits(&bs, (n)))

gzip parse_gzip(arena* arena, string8 file) {
    gzip out = { 
        .valid = true
    };

    bitstream bs = {
        .data = file.str,
        .num_bytes = file.size
    };
    
    if (BITS(8) != 31 || BITS(8) != 139) {
        log_error("File does not have valid gzip identifier");
        out.valid = false;
        return out;
    }

    u32 cm = BITS(8);
    if (cm != 8) {
        log_errorf("Unsupported compression type %d, must be 8", cm);
        out.valid = false;
        return out;
    }
    u32 flg = BITS(8);
    // timestamp
    BITS(16); BITS(16);
    // XFL + OS
    BITS(16);

    if (flg & FLG_EXTRA) { log_error("todo: extra"); }
    if (flg & FLG_NAME) { 
        out.name = str8_from_cstr(bs_get_ptr(&bs));
        char c = (char)BITS(8);
        while (c != 0) { c = (char)BITS(8); }
    }
    if (flg & FLG_COMMENT) { log_error("todo: comment"); }
    if (flg & FLG_HCRC) { log_error("todo: hcrc"); }

    u32 isize = *(u32*)(file.str + file.size - 4);
    // NOTE: the file could technically be more than 2^32 bytes
    // but that would require more dynamic memory allocation

    out.size = isize;
    out.data = CREATE_ARRAY(arena, u8, out.size);

    parse_deflate(&bs, out.data, out.size);
    
    BS_FLUSH_BYTE(&bs);

    u32 crc = BS_U32(&bs);
    u32 isize2 = BS_U32(&bs);
    log_debugf("%u %u", isize, isize2);

    return out;
}