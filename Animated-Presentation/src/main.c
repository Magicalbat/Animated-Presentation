#include <stdio.h>

#include "base/base.h"
#include "os.h"

int main() {
    arena_t* arena = arena_create(os_mem_pagesize() * 4);

    uint64_t buff_size = os_mem_pagesize() * 2;
    uint8_t* buff = (uint8_t*)arena_malloc(arena, buff_size);
    buff[0]  = 'h';
    buff[1]  = 'e';
    buff[2]  = 'l';
    buff[3]  = 'l';
    buff[4]  = 'o';
    buff[5]  = ' ';
    buff[6]  = 'w';
    buff[7]  = 'o';
    buff[8]  = 'r';
    buff[9]  = 'l';
    buff[10] = 'd';
    buff[11] = '\0';
    printf("%s\n", buff);

    arena_pop(arena, buff_size + 8);

    // Testing when the seg fault happens, seems to be working 
    for (int i = 0; i < buff_size; i++) {
        printf("%d\n", i);
        buff[i] = 0;
    }
    
    arena_free(arena);

	return 0;
}
