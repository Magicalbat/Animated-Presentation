#include <stdio.h>

#include "base/base.h"

int main() {
	// Page size of my pc is 4096
    
    printf("program init\n");

	arena_t* arena = arena_create(4096);

    printf("arena made\n");

	string8_t str = string8_create(arena, 128);
	memset(str.str, 'a', str.len);
	printf("%s\n", str.str);
    
    printf("string made\n");

	arena_free(arena);
    
    printf("arena free\n");
	
	return 0;
}


/*
#ifdef AP_PLATFORM_LINUX

#include <sys/mman.h>

#include "os.h"

//https://stackoverflow.com/questions/2782628/any-way-to-reserve-but-not-commit-memory-in-linux
//https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/mmap.html

void* os_mem_reserve(uint64_t size) {
	void* out = mmap(0, size, PROT_NONE, MAP_ANONYMOUS, -1, 0);
	return out;
}
void os_mem_commit(void* ptr, uint64_t size) {
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}
void os_mem_decommit(void* ptr, uint64_t size) {
    mprotect(ptr, size, PROT_NONE);
}
void os_mem_release(void* ptr, uint64_t size) {
    munmap(ptr, size);
}

#endif 
*/