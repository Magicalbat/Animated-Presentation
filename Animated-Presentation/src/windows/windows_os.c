#ifdef AP_PLATFORM_WINDOWS

#include <Windows.h>

#include "base/base.h"
#include "os.h"

void* os_mem_reserve(uint64_t size) {
	void* out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	return out;
}
void os_mem_commit(void* ptr, uint64_t size) {
	VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}
void os_mem_decommit(void* ptr, uint64_t size) {
	VirtualFree(ptr, size, MEM_DECOMMIT);
}
void os_mem_release(void* ptr, uint64_t size) {
	VirtualFree(ptr, 0, MEM_RELEASE);
}

uint64_t os_mem_pagesize() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwPageSize;
}

#endif