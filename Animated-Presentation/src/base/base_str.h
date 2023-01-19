#ifndef BASE_STR_H
#define BASE_STR_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "base_defs.h"
#include "base_mem.h"

// This is heavily based on the string 
// header in Mr 4th programming series: 
// https://github.com/Mr-4th-Programming/mr4th/blob/main/src/base/base_string.h

typedef struct {
    u8* str;
    u64 size;
} string8;

typedef struct {
    u16* str;
    u64 size;
} string16;

typedef struct {
    u32* str;
    u64 size;
} string32;

typedef struct string8_node {
    struct string8_node* next;
    string8 str;
} string8_node;

typedef struct {
    string8_node* first;
    string8_node* last;
    u64 node_count;
    u64 total_size;
} string8_list; 

typedef struct {
    string8 pre;
    string8 inbetween;
    string8 post;
} string8_join;

typedef struct {
    u32 code_point;
    u32 size;
} string_decode;

#define STR8_LIT(s) ((string8){ (u8*)(s), sizeof(s)-1 })

string8 str8_create(u8* str, u64 size);
string8 str8_from_range(u8* start, u8* end);
string8 str8_from_cstr(u8* cstr);
string8 str8_copy(arena* arena, string8 str);

b8 str8_equals(string8 a, string8 b);
b8 str8_contains(string8 a, string8 b);

u64 str8_find_first(string8 a, string8 b);
u64 str8_find_last(string8 a, string8 b);

string8 str8_prefix(string8 str, u64 size);
string8 str8_postfix(string8 str, u64 size);
string8 str8_substr(string8 str, u64 start, u64 end);
string8 str8_substr_size(string8 str, u64 start, u64 size);

void str8_list_push_existing(string8_list* list, string8 str, string8_node* node);
void str8_list_push(arena* arena, string8_list* list, string8 str);

string8_list str8_split(arena* arena, string8 orig, string8 split);
string8_list str8_split_char(arena* arena, string8 orig, u8 split_char);

string8 str8_concat(arena* arena, string8_list list);
string8 str8_join(arena* arena, string8_list list, string8_join join);

string8 str8_pushfv(arena* arena, const char* fmt, va_list args);
string8 str8_pushf(arena* arena, const char* fmt, ...);

string_decode str_decode_utf8(u8* str, u32 cap);
u32           str_encode_utf8(u8* dst, u32 code_point);

string_decode str_decode_utf16(u16* str, u32 cap);
u32           str_encode_utf16(u16* dst, u32 code_point);

string32 str32_from_str8(arena* arena, string8 str);
string8  str8_from_str32(arena* arena, string32 str);
string16 str16_from_str8(arena* arena, string8 str);
string8  str8_from_str16(arena* arena, string16 str);

#endif // BASE_STR_H
