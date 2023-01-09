#include "parse/parse.h"

#include "base/base.h"

// Fast decode path uses the method from stb_image.h zlib algorithm
// https://github.com/nothings/stb/blob/master/stb_image.h

// Fallback is modeled after puff.c, from the zlib contribs
// https://github.com/madler/zlib/blob/master/contrib/puff/puff.c

#define MAX_BITS 15
#define NUM_SYMS 288

#define FAST_BITS 9
#define FAST_MASK ((1 << FAST_BITS) - 1)

typedef struct {
    bitstream_t* bs;
    u8* out;
    u64 out_size;
    u64 out_pos;
} dstate_t;

typedef struct {
    u16 fast[1 << FAST_BITS];

    u16 counts[MAX_BITS + 1];
    u16 syms[NUM_SYMS];
    i32 fallback_index;
    i32 fallback_first;
} dhuffman_t;

typedef struct {
    u8* data;
    u32 size;
} u8arr_t;

static u16 reverse_u16(u16 n) {
    u16 o = n;
    o = ((o & 0xAAAA) >> 1) | ((o & 0x5555) << 1);
    o = ((o & 0xCCCC) >> 2) | ((o & 0x3333) << 2);
    o = ((o & 0xF0F0) >> 4) | ((o & 0x0F0F) << 4);
    o = ((o & 0xFF00) >> 8) | ((o & 0x00FF) << 8);
    return o;
}

#define reverse_bits(n, bits) (reverse_u16(n) >> (16 - bits))

// TODO: return success value
static void dhuffman_build(dhuffman_t* out, u8arr_t code_lens) {
    memset(out->fast, 0, sizeof(out->fast));
    memset(out->counts, 0, sizeof(out->counts));
    memset(out->syms, 0, sizeof(out->syms));

    for (u32 i = 0; i < code_lens.size; i++) {
        out->counts[code_lens.data[i]]++;
    }

    u16 next_code[MAX_BITS + 1] = { 0 };
    
    u16 code = 0;
    out->counts[0] = 0;
    for (u32 bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + out->counts[bits - 1]) << 1;
        next_code[bits] = code;
    }

    u16 offsets[MAX_BITS + 1] = { 0 };
    offsets[1] = 0;
    for (u32 i = 1; i < MAX_BITS; i++) {
        offsets[i + 1] = offsets[i] + out->counts[i];
    }

    for (u32 sym = 0; sym < code_lens.size; sym++) {
        u32 s = code_lens.data[sym];
        if (s != 0) {
            // fast
            if (s <= FAST_BITS) {
                u16 fast = (u16)((s << FAST_BITS) | sym);
                u16 i = reverse_bits(next_code[s], s);
                while (i < (1 << FAST_BITS)) {
                    out->fast[i] = fast;
                    i += (1 << s);
                }
                
                next_code[s]++;
            }
            
            // fallback
            out->syms[offsets[s]++] = sym;
        }
    }

    out->fallback_index = 0;
    out->fallback_first = 0;
    u16* count = out->counts + 1;
    for (u32 i = 0; i < FAST_BITS; i++) {
        out->fallback_index += *count;
        out->fallback_first += *count;

        out->fallback_first <<= 1;

        count++;
    }
}

#define PBITS(n) (bs_peek_bits(state->bs, (n)))
#define BITS(n)  (bs_get_bits (state->bs, (n)))

static void parse_stored(dstate_t* state) {
    log_error("TODO: stored block");
}

static u32 dhuffman_decode(dstate_t* state, dhuffman_t* huff) {
    u32 start_bits = PBITS(FAST_BITS);

    // fast
    u16 fast = huff->fast[start_bits];
    if (fast != 0) {
        u16 size = fast >> FAST_BITS;
        state->bs->bit_pos += size;

        return fast & FAST_MASK;
    }

    // fallback
    state->bs->bit_pos += FAST_BITS;

    i32 index = huff->fallback_index;
    i32 first = huff->fallback_first;
    i32 code = reverse_bits(start_bits, FAST_BITS) << 1;

    i32 count = 0;
    u16* next_count = huff->counts + 1 + FAST_BITS;

    for (u32 i = FAST_BITS; i < MAX_BITS; i++) {
        code |= BITS(1);
        count = *next_count++;

        if (code - count < first) {
            return huff->syms[index + (code - first)];
        }

        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }

    log_error("Invalid huffman code");
    return -1;
}

static const u16 lens_base[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const u16 lens_ext[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static const u16 dists_base[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577
};
static const u16 dists_extra[30] = { 
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
    12, 12, 13, 13
};

static void parse_codes(dstate_t* state, dhuffman_t len_huff, dhuffman_t dist_huff) {

}

static u8 fixed_len_lens[NUM_SYMS] = {
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};
static u8 fixed_dist_lens[32] = {
   5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};

static void parse_fixed(dstate_t* state) {
    log_debug("fixed");

    dhuffman_t len_huff = { 0 };
    dhuffman_t dist_huff = { 0 };
    dhuffman_build(&len_huff, (u8arr_t){ fixed_len_lens, STATIC_ARR_LEN(fixed_len_lens) });
    dhuffman_build(&dist_huff, (u8arr_t){ fixed_dist_lens, STATIC_ARR_LEN(fixed_dist_lens) });

    while (1) {
        u32 sym = dhuffman_decode(state, &len_huff);
        log_debugf("%u %c", sym, (char)sym);
        if (sym == 256) {
            log_debug("End of block");
            break;
        }
    }
}
static void parse_dynamic(dstate_t* state) {
    log_error("TODO: dynamic block");
}

void parse_deflate(bitstream_t* bs, u8* out, u64 out_size) {
    dstate_t state = {
        bs, out, out_size, 0
    };
    
    u32 last_block = bs_get_bits(state.bs, 1);

    do {
        u32 block_type = bs_get_bits(state.bs, 2);
        switch (block_type) {
            case 0: parse_stored (&state); break;
            case 1: parse_fixed  (&state); break;
            case 2: parse_dynamic(&state); break;
            default: log_errorf("Inalid deflate block %u", block_type); break;
        }
    } while (last_block != 1);
}