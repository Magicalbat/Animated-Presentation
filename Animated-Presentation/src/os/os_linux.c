#ifdef AP_PLATFORM_LINUX

#include <stddef.h>

#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#include "os.h"

arena_t*       lnx_arena;
string8_list_t lnx_cmd_args;

void os_main_init(int argc, char** argv) {
    lnx_arena = arena_create(KB(4));

    for (i32 i = 0; i < argc; i++) {
        string8_t str = str8_from_cstr(argv[i]);
        str8_list_push(lnx_arena, &lnx_cmd_args, str);
    }
}
void os_main_quit() {
    arena_destroy(lnx_arena);
}
string8_list_t os_get_cmd_args() {
    return lnx_cmd_args;
}

void* os_mem_reserve(u64 size) {
	void* out = mmap(NULL, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	return out;
}
void os_mem_commit(void* ptr, u64 size) {
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}
void os_mem_decommit(void* ptr, u64 size) {
    mprotect(ptr, size, PROT_NONE);
}
void os_mem_release(void* ptr, u64 size) {
    munmap(ptr, size);
}

uint64_t os_mem_pagesize() {
    return sysconf(_SC_PAGE_SIZE);
}

u64 os_now_microseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
void os_sleep_milliseconds(u32 t) {
    usleep(t * 1000);
}

#endif 
