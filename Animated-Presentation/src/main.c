#include <stdio.h>

#include "base/base.h"
#include "os/os.h"

int main() {
    arena_t* arena = arena_create(os_mem_pagesize() * 4);

    u64 buff_size = os_mem_pagesize() * 2;
    u8* buff = (u8*)arena_malloc(arena, buff_size);
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
    
    arena_free(arena);

	return 0;
}
