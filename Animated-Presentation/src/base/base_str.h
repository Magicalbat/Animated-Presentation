#ifndef BASE_STR_H
#define BASE_STR_H

#include <string.h>

#include "base_def.h"
#include "base_mem.h"

// This is heavily based on the string 
// header in Mr 4th programming series: 
// https://github.com/Mr-4th-Programming/mr4th/blob/main/src/base/base_string.h

typedef struct {
    u8* str;
    u64 size;
} string8_t;

typedef struct string8_node {
    struct string8_node* next;
    string8_t str;
} string8_node_t;

typedef struct {
    string8_node_t* first;
    string8_node_t* last;
    u64 node_count;
    u64 total_size;
} string8_list_t; 

typedef struct {
    string8_t pre;
    string8_t inbetween;
    string8_t post;
} string8_join_t;

string8_t str8_create(u8* str, u64 size);
string8_t str8_from_range(u8* start, u8* end);
string8_t str8_from_cstr(u8* cstr);
string8_t str8_copy(arena_t* arena, string8_t str);

b8 str8_equals(string8_t a, string8_t b);
b8 str8_contains(string8_t a, string8_t b);

u64 str8_find_first(string8_t a, string8_t b);
u64 str8_find_last(string8_t a, string8_t b);

#define str8_lit(s) ((string8_t){ (u8*)(s), sizeof(s)-1 })

string8_t str8_prefix(string8_t str, u64 size);
string8_t str8_postfix(string8_t str, u64 size);
string8_t str8_substr(string8_t str, u64 start, u64 end);
string8_t str8_substr_size(string8_t str, u64 start, u64 size);

void str8_list_add_existing(string8_list_t* list, string8_t str, string8_node_t* node);
void str8_list_add(arena_t* arena, string8_list_t* list, string8_t str);

string8_list_t str8_split(arena_t* arena, string8_t orig, string8_t split);
string8_list_t str8_split_char(arena_t* arena, string8_t orig, u8 split_char);
string8_t str8_concat(arena_t* arena, string8_list_t list);
string8_t str8_join(arena_t* arena, string8_list_t list, string8_join_t join);

#endif // BASE_STR_H