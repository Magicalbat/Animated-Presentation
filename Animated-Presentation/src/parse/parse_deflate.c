#include "parse/parse.h"

#include "base/base.h"

// Fast decode path uses the method from stb_image.h zlib algorithm
// Fallback is modeled after puff.c, from the zlib contribs

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
} dhuffman_t;

typedef struct {
    u8* data;
    u32 size;
} u8arr_t;

// TODO: return success value
void build_huffman(dhuffman_t* out, u8arr_t code_lens) {
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
            if (s < FAST_BITS) {
                // TODO: might have to reverse this
                u16 fast = (u16)((s << 9) | sym);
                u32 i = next_code[s];
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
}

#define BITS(n) (bs_get_bits(state->bs, (n)))

void parse_stored(dstate_t* state) {
    log_error("TODO: stored block");
}
void parse_fixed(dstate_t* state) {
    dhuffman_t len_huff = { 0 };
    
    u8 lens[288] = { 0 };
    u32 sym = 0;
    for (; sym < 144; sym++) lens[sym] = 8;
    for (; sym < 256; sym++) lens[sym] = 9;
    for (; sym < 280; sym++) lens[sym] = 7;
    for (; sym < 288; sym++) lens[sym] = 9;
    dhuffman_t huff = { 0 };
    build_huffman(&huff, (u8arr_t){
        lens, 288
    });
}
void parse_dynamic(dstate_t* state) {
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