#ifdef AP_PLATFORM_LINUX

#include <stddef.h>

#include <sys/mman.h>
#include <unistd.h>

#include "os.h"

//https://stackoverflow.com/questions/2782628/any-way-to-reserve-but-not-commit-memory-in-linux
//https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/mmap.html

void* os_mem_reserve(uint64_t size) {
	void* out = mmap(NULL, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    ASSERT(out != MAP_FAILED && "Failed to allocate memory");
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

uint64_t os_mem_pagesize() {
    return sysconf(_SC_PAGE_SIZE);
}

#endif 