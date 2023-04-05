#ifndef BASE_STR_H
#define BASE_STR_H

#ifdef __cplusplus
extern "C" { 
#endif

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
    string8_list names;

    u64 num_strings;
    string8* strings;
} string8_registry;

typedef struct {
    u32 code_point;
    u32 size;
} string_decode;

#define STR8_LIT(s) ((string8){ (u8*)(s), sizeof(s)-1 })
#define STR(s) STR8_LIT(s)

AP_EXPORT string8 str8_create(u8* str, u64 size);
AP_EXPORT string8 str8_from_range(u8* start, u8* end);
AP_EXPORT string8 str8_from_cstr(u8* cstr);

AP_EXPORT string8 str8_copy(marena* arena, string8 str);
AP_EXPORT u8*     str8_to_cstr(marena* arena, string8 str);

AP_EXPORT b8 str8_equals(string8 a, string8 b);
AP_EXPORT b8 str8_contains(string8 a, string8 b);

AP_EXPORT u64 str8_find_first(string8 a, string8 b);
AP_EXPORT u64 str8_find_last(string8 a, string8 b);

AP_EXPORT string8 str8_prefix(string8 str, u64 size);
AP_EXPORT string8 str8_postfix(string8 str, u64 size);
AP_EXPORT string8 str8_substr(string8 str, u64 start, u64 end);
AP_EXPORT string8 str8_substr_size(string8 str, u64 start, u64 size);

AP_EXPORT string8 str8_cut_end_until(string8 str, u8 c);

AP_EXPORT void str8_list_push_existing(string8_list* list, string8 str, string8_node* node);
AP_EXPORT void str8_list_push(marena* arena, string8_list* list, string8 str);

AP_EXPORT string8_list str8_split(marena* arena, string8 orig, string8 split);
AP_EXPORT string8_list str8_split_char(marena* arena, string8 orig, u8 split_char);

AP_EXPORT string8 str8_concat(marena* arena, string8_list list);
AP_EXPORT string8 str8_join(marena* arena, string8_list list, string8_join join);

AP_EXPORT string8 str8_pushfv(marena* arena, const char* fmt, va_list args);
AP_EXPORT string8 str8_pushf(marena* arena, const char* fmt, ...);

AP_EXPORT u64     str8_reg_push(marena* arena, string8_registry* reg, string8 name);
AP_EXPORT void    str8_reg_init_arr(marena* arena, string8_registry* reg);
AP_EXPORT string8 str8_reg_get(string8_registry* reg, u64 id);
AP_EXPORT void    str8_reg_clear(string8_registry* reg);

AP_EXPORT string_decode str_decode_utf8(u8* str, u32 cap);
AP_EXPORT u32           str_encode_utf8(u8* dst, u32 code_point);

AP_EXPORT string_decode str_decode_utf16(u16* str, u32 cap);
AP_EXPORT u32           str_encode_utf16(u16* dst, u32 code_point);

AP_EXPORT string32 str32_from_str8(marena* arena, string8 str);
AP_EXPORT string8  str8_from_str32(marena* arena, string32 str);
AP_EXPORT string16 str16_from_str8(marena* arena, string8 str);
AP_EXPORT string8  str8_from_str16(marena* arena, string16 str);

#ifdef __cplusplus
}
#endif

#endif // BASE_STR_H
