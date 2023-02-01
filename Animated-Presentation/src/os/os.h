#ifndef OS_H
#define OS_H

#include "base/base.h"

#if defined(_WIN32)
    #ifndef UNICODE
        #define UNICODE
    #endif
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
} file_flags;

typedef struct {
    // TODO: Time stuff
    u64 size;
    file_flags flags;
} file_stats;

typedef enum {
    FOPEN_READ,
    FOPEN_WRITE,
    FOPEN_APPEND
} file_mode;

typedef struct {
    #if defined(_WIN32)
        HANDLE file_handle;
    #elif defined(__linux__)
        int fd;
    #endif
} os_file;

typedef struct {
    #if defined(_WIN32)
        HMODULE module;
    #elif defined(__linux__)
        void* handle;
    #endif
} os_library;

typedef void (*void_func)(void);

void         os_main_init(int argc, char** argv);
void         os_main_quit(void);
string8_list os_get_cmd_args(void);

void* os_mem_reserve(u64 size);
void  os_mem_commit(void* ptr, u64 size);
void  os_mem_decommit(void* ptr, u64 size);
void  os_mem_release(void* ptr, u64 size);

u64 os_mem_pagesize(void);

// TODO: make sure linux and windows timestamps are consistent
datetime os_now_localtime(void);

u64  os_now_microseconds(void);
void os_sleep_milliseconds(u32 t);

string8    os_file_read(arena* arena, string8 path);
b32        os_file_write(string8 path, string8_list str_list);
b32        os_file_append(string8 path, string8_list str_lit);
file_stats os_file_get_stats(string8 path);

os_file os_file_open(string8 path, file_mode open_mode);
b32     os_file_write_open(os_file file, string8 str);
void    os_file_close(os_file file);

os_library os_lib_load(string8 path);
void_func  os_lib_func(os_library lib, const char* func_name);
void       os_lib_release(os_library lib);

#endif // OS_H
