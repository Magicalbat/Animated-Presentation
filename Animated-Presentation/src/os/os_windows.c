#ifdef AP_PLATFORM_WINDOWS

#include <Windows.h>

#include "base/base.h"
#include "os.h"

static arena_t*       w32_arena;
static string8_list_t w32_cmd_args;

void os_main_init(int argc, char** argv) {
    w32_arena = arena_create(KB(4));

    for (i32 i = 0; i < argc; i++) {
        string8_t str = str8_from_cstr(argv[i]);
        str8_list_push(w32_arena, &w32_cmd_args, str);
    }
}
void os_main_quit() {
    arena_free(w32_arena);
}
string8_list_t os_get_cmd_args() {
    return w32_cmd_args;
}

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
