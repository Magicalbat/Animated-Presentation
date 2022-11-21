#include <stdio.h>

#include "os/os.h"
#include "base/base.h"

void test_print_fn(const char* name, string8_t str) {
    printf("%s: %.*s, size: %lu\n", name, (i32)str.size, str.str, str.size);
}

#define test_print(str) test_print_fn(#str, str)
#define bool_print(b) (b ? "true" : "false")

int main() {
    arena_t* arena = arena_create(os_mem_pagesize() * 4);

    vec3_t v1 = (vec3_t){ 1.0f, 20.0f, 5.7f };
    vec3_t v2 = vec3_nrm(v1);

    printf("( %f, %f, %f )\n", v2.x, v2.y, v2.z);

    u8* test_cstr = "Hello";

    string8_t create = str8_create(test_cstr, 5);
    string8_t range = str8_from_range(test_cstr, test_cstr + 2);
    string8_t cstr = str8_from_cstr(test_cstr);

    test_print(create);
    test_print(range);
    test_print(cstr);

    string8_t test_str =  str8_lit("Hello again world");
    string8_t test_str2 = str8_lit("Hello ... again ... world");

    printf("str equals str2: %s\n", bool_print(str8_equals(test_str, test_str2)));
    printf("str contains \"world\": %s\n", bool_print(str8_contains(test_str, str8_lit("world"))));
    printf("str contains \"AGAIN\": %s\n", bool_print(str8_contains(test_str, str8_lit("AGAIN"))));
    printf("str find first \"world\": %lu\n", str8_find_first(test_str, str8_lit("world")));
    printf("str find last \" \": %lu\n", str8_find_last(test_str, str8_lit(" ")));

    string8_t pre = str8_prefix(test_str, 5);
    string8_t post = str8_postfix(test_str, 5);
    string8_t substr = str8_substr(test_str, 0, 8);
    string8_t substr_size = str8_substr_size(test_str, 0, 2);

    test_print(pre);
    test_print(post);
    test_print(substr);
    test_print(substr_size);

    string8_list_t list = str8_split(arena, test_str2, str8_lit(" ... "));
    FOR_SLL(string8_node_t, list.first, node) {
        test_print(node->str);
    }

    char split = ' ';
    list = str8_split_char(arena, test_str, split);
    FOR_SLL(string8_node_t, list.first, node) {
        test_print(node->str);
    }

    string8_t joined = str8_join(arena, list, (string8_join_t){
        .pre = str8_lit("Join: "),
        .inbetween = str8_lit(" - "),
        .post = str8_lit(" end")
    });
    test_print(joined);

    string8_t concat = str8_concat(arena, list);
    test_print(concat);

    string8_t copy = str8_copy(arena, test_str);
    test_print(copy);

    arena_free(arena);

	return 0;
}
