#include "base_str.h"

string8_t str8_create(u8* str, u64 size) {
    return (string8_t){ str, size };
}
string8_t str8_from_range(u8* start, u8* end) {
    return (string8_t){ start, (u64)(end - start) };
}
string8_t str8_from_cstr(u8* cstr) {
    u8* ptr = cstr;
    for(; *ptr != 0; ptr += 1);
    return str8_from_range(cstr, ptr);
}
string8_t str8_copy(arena_t* arena, string8_t str) {
    string8_t out = { 
        .str = arena_alloc(arena, str.size),
        .size = str.size
    };

    memcpy(out.str, str.str, str.size);
    
    return out;
}

bool str8_equals(string8_t a, string8_t b) {
    if (a.size != b.size)
        return false;

    for (u64 i = 0; i < a.size; i++)  {
        if (a.str[i] != b.str[i])
            return false;
    }
    
    return true;
}

string8_t str8_prefix(string8_t str, u64 size) {
    u64 clamped_size = MIN(size, str.size);
    return (string8_t){ str.str, clamped_size };
}
string8_t str8_postfix(string8_t str, u64 size) {
    u64 clamped_size = MIN(str.size, size);
    u64 new_pos = str.size - clamped_size;
    return (string8_t){ str.str + new_pos, clamped_size };
}
string8_t str8_substr(string8_t str, u64 start, u64 end) {
    u64 end_clamped = MIN(str.size, end);
    u64 start_clamped = MIN(start, end_clamped);
    return (string8_t){ str.str + start_clamped, end_clamped - start_clamped };
}
string8_t str8_substr_size(string8_t str, u64 start, u64 size) {
    return str8_substr(str, start, start + size);
}

void str8_list_add_existing(string8_list_t* list, string8_t str, string8_node_t* node) {
    node->str = str;
    SLL_PUSH_BACK(list->first, list->last, node);
    list->node_count++;
    list->total_size += str.size;
}
void str8_list_add(arena_t* arena, string8_list_t* list, string8_t str) {
    string8_node_t* node = CREATE_ZERO_STRUCT(node, string8_node_t, arena);
    str8_list_add_existing(list, str, node);
}
string8_list_t str8_split(arena_t* arena, string8_t str, string8_t split) {
    string8_list_t list_out = (string8_list_t){ .total_size = 0 };

    u8* ptr = str.str;
    u8* word_first = ptr;
    u8* end = ptr + str.size - split.size;
    for (; ptr < end; ptr += 1) {
        bool split_found = true;
        for (u64 i = 0; i < split.size; i++) {
            if (ptr[i] != split.str[i]) {
                split_found = false;
                break;
            }
        }

        if (split_found) {
            if (word_first < ptr) {
                str8_list_add(arena, &list_out, str8_from_range(word_first, ptr));
            }
            word_first = ptr + split.size;
        }
    }

    if (word_first <= ptr) {
        str8_list_add(arena, &list_out, str8_from_range(word_first, ptr + split.size));
    }

    return list_out;
}
string8_list_t str8_split_char(arena_t* arena, string8_t str, u8 split_char) {
    string8_t char_str = (string8_t){ .str=&split_char, .size=1 };
    return str8_split(arena, str, char_str);
}
string8_t str8_concat(arena_t* arena, string8_list_t list) {
    string8_t out = {
        .str = arena_alloc(arena, list.total_size),
        .size = list.total_size
    };

    u8* ptr = out.str;

    for (string8_node_t* node = list.first; node != NULL; node = node->next) {
        memcpy(ptr, node->str.str, node->str.size);
        ptr += node->str.size;
    }

    return out;
}
string8_t str8_join(arena_t* arena, string8_list_t list, string8_join_t join) {
    u64 out_size = join.pre.size + join.inbetween.size * (list.node_count - 1) + list.total_size + join.post.size + 1;
    
    string8_t out = {
        .str = arena_alloc(arena, out_size),
        .size = out_size
    };

    memcpy(out.str, join.pre.str, join.pre.size);

    u8* ptr = out.str + join.pre.size;

    for (string8_node_t* node = list.first; node != NULL; node = node->next) {
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