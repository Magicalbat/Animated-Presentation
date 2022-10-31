#include <stdio.h>

#include "base/base.h"

int main() {
	// Page size of my pc is 4096

	arena_t* arena = arena_create(4096 * 2);

	string8_t str = string8_create(arena, 2048);
	memset(str.str, 'a', str.len);
	printf("%s\n", str.str);

	arena_free(arena);
	
	return 0;
}
