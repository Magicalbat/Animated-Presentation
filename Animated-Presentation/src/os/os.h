#ifndef OS_H
#define OS_H

#include "base/base.h"

#if defined(AP_PLATFORM_WINDOWS)
    #if !defined(UNICODE)
        #define UNICODE
    #endif
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <timeapi.h>
#elif defined(AP_PLATFORM_LINUX)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <time.h>
#endif

typedef enum {
    FILE_IS_DIR = (1 << 0)
} file_flags_t;

typedef struct {
    // TODO: Time stuff
    u64 size;
    file_flags_t flags;
} file_stats_t;

void           os_main_init(int argc, char** argv);
void           os_main_quit();
string8_list_t os_get_cmd_args();

void* os_mem_reserve(u64 size);
void  os_mem_commit(void* ptr, u64 size);
void  os_mem_decommit(void* ptr, u64 size);
void  os_mem_release(void* ptr, u64 size);

u64 os_mem_pagesize();

u64  os_now_microseconds();
void os_sleep_milliseconds(u32 t);

string8_t    os_file_read(arena_t* arena, string8_t path);
void         os_file_write(string8_t path, string8_list_t str_list);
file_stats_t os_file_get_stats(string8_t path);

// TODO
// Append to file?
// Load module
// Get datetime

#endif // OS_H
