#ifdef AP_PLATFORM_WINDOWS

#include <Windows.h>

#include "base/base.h"
#include "os.h"

void* os_mem_reserve(u64 size) {
	void* out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	return out;
}
void os_mem_commit(void* ptr, u64 size) {
	VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}
void os_mem_decommit(void* ptr, u64 size) {
	VirtualFree(ptr, size, MEM_DECOMMIT);
}
void os_mem_release(void* ptr, u64 size) {
	VirtualFree(ptr, 0, MEM_RELEASE);
}

u64 os_mem_pagesize() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwPageSize;
}

#endif
