#ifndef OS_H
#define OS_H

#include "base/base.h"

#if defined(_WIN32)
    #define UNICODE
    #define WIN32_LEAN_AND_MEAN

    #include <Windows.h>
    #include <timeapi.h>
#elif defined(__linux__)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <time.h>
    #include <string.h>
    #include <errno.h>
    #include <dlfcn.h>
#endif

typedef enum {
    FILE_IS_DIR = (1 << 0)
} file_flags_t;

typedef struct {
    // TODO: Time stuff
    u64 size;
    file_flags_t flags;
} file_stats_t;

typedef enum {
    FOPEN_READ,
    FOPEN_WRITE,
    FOPEN_APPEND
} file_mode_t;

typedef struct {
    #if defined(_WIN32)
        HANDLE file_handle;
    #elif defined(__linux__)
        int fd;
    #endif
} os_file_t;

typedef struct {
    #if defined(_WIN32)
    #elif defined(__linux__)
        void* handle;
    #endif
} os_library_t;

typedef void (*void_func_t)(void);

void           os_main_init(int argc, char** argv);
void           os_main_quit();
string8_list_t os_get_cmd_args();

void* os_mem_reserve(u64 size);
void  os_mem_commit(void* ptr, u64 size);
void  os_mem_decommit(void* ptr, u64 size);
void  os_mem_release(void* ptr, u64 size);

u64 os_mem_pagesize();

// TODO: make sure linux and windows timestamps are consistent
datetime_t os_now_localtime();

u64  os_now_microseconds();
void os_sleep_milliseconds(u32 t);

string8_t    os_file_read(arena_t* arena, string8_t path);
b32          os_file_write(string8_t path, string8_list_t str_list);
b32          os_file_append(string8_t path, string8_list_t str_lit);
file_stats_t os_file_get_stats(string8_t path);

os_file_t os_file_open(string8_t path, file_mode_t open_mode);
b32       os_file_write_open(os_file_t file, string8_t str);
void      os_file_close(os_file_t file);

os_library_t os_lib_load(string8_t path);
void_func_t  os_lib_func(os_library_t lib, string8_t func_name);
void         os_lib_release(os_library_t lib);

// TODO
// Load module
// Get datetime

#endif // OS_H
