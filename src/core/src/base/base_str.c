#include "base/base_str.h"
#include "base/base_log.h"

string8 str8_create(u8* str, u64 size) {
    return (string8){ str, size };
}
string8 str8_from_range(u8* start, u8* end) {
    return (string8){ start, (u64)(end - start) };
}
string8 str8_from_cstr(u8* cstr) {
    u8* ptr = cstr;
    for(; *ptr != 0; ptr += 1);
    return str8_from_range(cstr, ptr);
}

string8 str8_copy(marena* arena, string8 str) {
    string8 out = { 
        .str = (u8*)marena_push(arena, str.size),
        .size = str.size
    };

    memcpy(out.str, str.str, str.size);
    
    return out;
}
u8* str8_to_cstr(marena* arena, string8 str) {
    u8* out = CREATE_ARRAY(arena, u8, str.size + 1);
    
    memcpy(out, str.str, str.size);
    out[str.size] = '\0';

    return out;
}

b8 str8_equals(string8 a, string8 b) {
    if (a.size != b.size)
        return false;

    for (u64 i = 0; i < a.size; i++)  {
        if (a.str[i] != b.str[i])
            return false;
    }
    
    return true;
}
b8 str8_contains(string8 a, string8 b) {
    for (u64 i = 0; i < a.size - b.size + 1; i++) {
        b8 contains = true;
        for (u64 j = 0; j < b.size; j++) {
            if (a.str[i + j] != b.str[j]) {
                contains = false;
                break;
            }
        }

        if (contains) {
            return true;
        }
    }

    return false;
}

u64 str8_find_first(string8 a, string8 b) {
    for (u64 i = 0; i < a.size - b.size + 1; i++) {
        b8 contains = true;
        for (u64 j = 0; j < b.size; j++) {
            if (a.str[i + j] != b.str[j]) {
                contains = false;
                break;
            }
        }

        if (contains) {
            return i;
        }
    }
    return (u64)(-1);
}
u64 str8_find_last(string8 a, string8 b) {
    u64 out = (u64)(-1);
    for (u64 i = 0; i < a.size - b.size + 1; i++) {
        b8 contains = true;
        for (u64 j = 0; j < b.size; j++) {
            if (a.str[i + j] != b.str[j]) {
                contains = false;
                break;
            }
        }

        if (contains) {
            out = i;
        }
    }
    return out;
}

string8 str8_prefix(string8 str, u64 size) {
    u64 clamped_size = MIN(size, str.size);
    return (string8){ str.str, clamped_size };
}
string8 str8_postfix(string8 str, u64 size) {
    u64 clamped_size = MIN(str.size, size);
    u64 new_pos = str.size - clamped_size;
    return (string8){ str.str + new_pos, clamped_size };
}
string8 str8_substr(string8 str, u64 start, u64 end) {
    u64 end_clamped = MIN(str.size, end);
    u64 start_clamped = MIN(start, end_clamped);
    return (string8){ str.str + start_clamped, end_clamped - start_clamped };
}
string8 str8_substr_size(string8 str, u64 start, u64 size) {
    return str8_substr(str, start, start + size);
}

string8 str8_cut_end_until(string8 str, u8 c) {
    string8 out = str;

    while (out.size > 0 && out.str[out.size - 1] != c) {
        out.size--;
    }

    if (out.size != 0)
        out.size--;

    return out;
}

void str8_list_push_existing(string8_list* list, string8 str, string8_node* node) {
    node->str = str;
    SLL_PUSH_BACK(list->first, list->last, node);
    list->node_count++;
    list->total_size += str.size;
}
void str8_list_push(marena* arena, string8_list* list, string8 str) {
    string8_node* node = CREATE_ZERO_STRUCT(arena, string8_node);
    str8_list_push_existing(list, str, node);
}
string8_list str8_split(marena* arena, string8 str, string8 split) {
    string8_list list_out = (string8_list){ .total_size = 0 };

    u8* ptr = str.str;
    u8* word_first = ptr;
    u8* end = ptr + str.size - split.size;
    for (;ptr < end; ptr += 1) {
        b8 split_found = true;
        for (u64 i = 0; i < split.size; i++) {
            if (ptr[i] != split.str[i]) {
                split_found = false;
                break;
            }
        }

        if (split_found) {
            if (word_first < ptr) {
                str8_list_push(arena, &list_out, str8_from_range(word_first, ptr));
            }
            word_first = ptr + split.size;
        }
    }

    if (word_first <= ptr) {
        str8_list_push(arena, &list_out, str8_from_range(word_first, ptr + split.size));
    }

    return list_out;
}
string8_list str8_split_char(marena* arena, string8 str, u8 split_char) {
    string8 char_str = (string8){ .str=&split_char, .size=1 };
    return str8_split(arena, str, char_str);
}

string8 str8_concat(marena* arena, string8_list list) {
    string8 out = {
        .str = (u8*)marena_push(arena, list.total_size),
        .size = list.total_size
    };

    u8* ptr = out.str;

    for (string8_node* node = list.first; node != NULL; node = node->next) {
        memcpy(ptr, node->str.str, node->str.size);
        ptr += node->str.size;
    }

    return out;
}
string8 str8_join(marena* arena, string8_list list, string8_join join) {
    u64 out_size = join.pre.size + join.inbetween.size * (list.node_count - 1) + list.total_size + join.post.size + 1;
    
    string8 out = {
        .str = (u8*)marena_push(arena, out_size),
        .size = out_size
    };

    memcpy(out.str, join.pre.str, join.pre.size);

    u8* ptr = out.str + join.pre.size;

    for (string8_node* node = list.first; node != NULL; node = node->next) {
        if (node != list.first) {
            memcpy(ptr, join.inbetween.str, join.inbetween.size);
            ptr += join.inbetween.size;
        }

        memcpy(ptr, node->str.str, node->str.size);
        ptr += node->str.size;
    }

    memcpy(ptr, join.post.str, join.post.size);
    ptr += join.post.size;

    *ptr = 0;

    return out;
}

string8 str8_pushfv(marena* arena, const char* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);

    u64 init_size = 1024;
    u8* buffer = CREATE_ARRAY(arena, u8, init_size);
    u64 size = vsnprintf((char*)buffer, init_size, fmt, args);

    string8 out = { 0 };
    if (size < init_size) {
        marena_pop(arena, init_size - size - 1);
        out = (string8){ buffer, size };
    } else {
        // NOTE: This path may not work
        marena_pop(arena, init_size);
        u8* fixed_buff = CREATE_ARRAY(arena, u8, size + 1);
        u64 final_size = vsnprintf((char*)fixed_buff, size + 1, fmt, args);
        out = (string8){ fixed_buff, final_size };
    }

    va_end(args2);

    return out;
}

string8 str8_pushf(marena* arena, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    string8 out = str8_pushfv(arena, fmt, args);

    va_end(args);

    return out;
}

u64 str8_reg_push(marena* arena, string8_registry* reg, string8 name) {
    u64 out = reg->names.node_count;

    reg->num_strings += 1;
    str8_list_push(arena, &reg->names, name);
    
    return out;
}
void str8_reg_init_arr(marena* arena, string8_registry* reg) {
    reg->strings = CREATE_ZERO_ARRAY(arena, string8, reg->num_strings);
}
string8 str8_reg_get(string8_registry* reg, u64 id) {
    if (id >= reg->num_strings) {
        log_errorf("Invalid string8 registry id %llu", id);
        return (string8){ 0 };
    }
    return reg->strings[id];
}
void str8_reg_clear(string8_registry* reg) {
    reg->num_strings = 0;

    reg->names = (string8_list){
        .first = NULL,
        .last = NULL,
    };
}

// https://github.com/skeeto/branchless-utf8/blob/master/utf8.h
// https://github.com/Mr-4th-Programming/mr4th/blob/main/src/base/base_string.cpp
string_decode str_decode_utf8(u8* str, u32 cap) {
    static u8 lengths[] = {
        1, 1, 1, 1, // 000xx
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        0, 0, 0, 0, // 100xx
        0, 0, 0, 0,
        2, 2, 2, 2, // 110xx
        3, 3,       // 1110x
        4,          // 11110
        0,          // 11111
    };
    static u8 first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
    static u8 final_shift[] = { 0, 18, 12, 6, 0 };

    string_decode out = { .size=0 };

    if (cap > 0) {
        out.code_point = '#';
        out.size = 1;
        
        u32 len = lengths[str[0] >> 3];
        if (len > 0 && len <= cap) {
            u32 code_point = (str[0] & first_byte_mask[len]) << 18;
            switch(len) {
                case 4: code_point |= (str[3] & 0b00111111) << 0;
                case 3: code_point |= (str[2] & 0b00111111) << 6;
                case 2: code_point |= (str[1] & 0b00111111) << 12;
                default: break;
            }
            code_point >>= final_shift[len];

            out.code_point = code_point;
            out.size = len;
        }
    }

    return out;
}

u32 str_encode_utf8(u8* dst, u32 code_point) {
    u32 size = 0;

    if (code_point < (1 << 8)) {
        dst[0] = (u8)code_point;
        size = 1;
    } else if (code_point < (1 << 11)) {
        dst[0] = 0b11000000 | (code_point >> 6);
        dst[1] = 0b10000000 | (code_point & 0b00111111);
        size = 2;
    } else if (code_point < (1 << 16)) {
        dst[0] = 0b11100000 | (code_point >> 12);
        dst[1] = 0b10000000 | ((code_point >> 6) & 0b00111111);
        dst[1] = 0b10000000 | (code_point & 0b00111111);
        size = 3;
    } else if (code_point < (1 << 21)) {
        dst[0] = 0b11110000 | (code_point >> 18);
        dst[1] = 0b10000000 | ((code_point >> 12) & 0b00111111);
        dst[2] = 0b10000000 | ((code_point >> 6) & 0b00111111);
        dst[3] = 0b10000000 | (code_point & 0b00111111);
        size = 4;
    } else {
        dst[0] = '#';
        size = 1;
    }

    return size;
}

// https://en.wikipedia.org/wiki/UTF-16
string_decode str_decode_utf16(u16* str, u32 cap) {
    string_decode out = { '#', 1 };
    u16 x = str[0];

    if (x < 0xd800 || x >= 0xdfff) {
        out.code_point = x;
    } else if (cap >= 2) {
        u16 y = str[1];
        if (x >= 0xd800 && x <= 0xdbff && y >= 0xdc00 && y <= 0xdfff) {
            u16 x2 = x - 0xd800;
            u16 y2 = y - 0xdc00;
            out.code_point = ((x2 << 10) | y2) + 0x10000;
            out.size = 2;
        }
    }

    return out;
}
u32 str_encode_utf16(u16* dst, u32 code_point) {
    u32 size = 0;

    if (code_point < 0x010000) {
        dst[0] = (u16)code_point;
        size = 1;
    } else {
        u32 u_p = code_point - 0x10000;
        dst[0] = 0xd800 + (u_p >> 10);
        dst[0] = 0xdc00 + (u_p & 0x3ff);
        size = 2;
    }

    return size;
}

string32 str32_from_str8(marena* arena, string8 str) {
    u32* buff = CREATE_ARRAY(arena, u32, str.size + 1);

    u32* ptr_out = buff;
    u8* ptr = str.str;
    u8* ptr_end = str.str + str.size;
    for(;ptr < ptr_end;){
        string_decode decode = str_decode_utf8(ptr, (u32)(ptr_end - ptr));

        *ptr_out = decode.code_point;

        ptr += decode.size;
        ptr_out += 1;
    }

    *ptr_out = 0;

    u64 alloc_count = str.size + 1;
    u64 string_count = (u64)(ptr_out - buff);
    u64 unused_count = alloc_count - string_count - 1;
    marena_pop(arena, unused_count * (sizeof(*buff)));

    return (string32){ .str = buff, .size = string_count };
}
string8 str8_from_str32(marena* arena, string32 str) {
    u8* buff = CREATE_ARRAY(arena, u8, str.size * 4 + 1);

    u8* ptr_out = buff;
    u32* ptr = str.str;
    u32* ptr_end = str.str + str.size;
    for(;ptr < ptr_end;){
        u32 encode_size = str_encode_utf8(ptr_out, *ptr);

        ptr_out += encode_size;
        ptr += 1;
    }

    *ptr_out = 0;

    u64 alloc_count = str.size * 4 + 1;
    u64 string_count = (u64)(ptr_out - buff);
    u64 unused_count = alloc_count - string_count - 1;
    marena_pop(arena, unused_count * (sizeof(*buff)));

    return (string8){ .str = buff, .size = string_count };
}
string16 str16_from_str8(marena* arena, string8 str) {
    u16* buff = CREATE_ARRAY(arena, u16, str.size * 2 + 1);

    u16* ptr_out = buff;
    u8* ptr = str.str;
    u8* ptr_end = str.str + str.size;
    for(;ptr < ptr_end;){
        string_decode decode = str_decode_utf8(ptr, (u32)(ptr_end - ptr));
        u32 encode_size = str_encode_utf16(ptr_out, decode.code_point);

        ptr += decode.size;
        ptr_out += encode_size;
    }

    *ptr_out = 0;

    u64 alloc_count = str.size * 2 + 1;
    u64 string_count = (u64)(ptr_out - buff);
    u64 unused_count = alloc_count - string_count - 1;
    marena_pop(arena, unused_count * (sizeof(*buff)));

    return (string16){ .str = buff, .size = string_count };

}
string8 str8_from_str16(marena* arena, string16 str) {
    u8* buff = CREATE_ARRAY(arena, u8, str.size * 4 + 1);

    u8* ptr_out = buff;
    u16* ptr = str.str;
    u16* ptr_end = str.str + str.size;
    for(;ptr < ptr_end;){
        string_decode decode = str_decode_utf16(ptr, (u32)(ptr_end - ptr));
        u16 encode_size = str_encode_utf8(ptr_out, decode.code_point);

        ptr_out += encode_size;
        ptr += decode.size;
    }

    *ptr_out = 0;

    u64 alloc_count = str.size * 4 + 1;
    u64 string_count = (u64)(ptr_out - buff);
    u64 unused_count = alloc_count - string_count - 1;
    marena_pop(arena, unused_count * (sizeof(*buff)));

    return (string8){ .str = buff, .size = string_count };
}
