#ifndef OS_H
#define OS_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"

#if defined(_WIN32)
    #ifndef UNICODE
        #define UNICODE
    #endif
    #define WIN32_LEAN_AND_MEAN

    #include <Windows.h>
    #include <timeapi.h>
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <time.h>
    #include <string.h>
    #include <errno.h>
    #include <dlfcn.h>

    #if defined(__EMSCRIPTEN__)
        #include <emscripten.h>
        #include <emscripten/fetch.h>
    #endif
#endif

typedef enum {
    FILE_IS_DIR = (1 << 0)
} file_flags;

typedef struct {
    u64 size;
    file_flags flags;
    datetime modify_time;
} file_stats;

typedef enum {
    FOPEN_READ,
    FOPEN_WRITE,
    FOPEN_APPEND
} file_mode;

typedef struct {
    #if defined(_WIN32)
        HANDLE file_handle;
    #elif defined(__linux__) || defined(__EMSCRIPTEN__)
        int fd;
    #endif
} os_file;

typedef struct {
    #if defined(_WIN32)
        HMODULE module;
    #elif defined(__linux__) || defined(__EMSCRIPTEN__)
        void* handle;
    #endif
} os_library;

typedef void (void_func)(void);

AP_EXPORT void         os_main_init(int argc, char** argv);
AP_EXPORT void         os_main_quit(void);
AP_EXPORT string8_list os_get_cmd_args(void);

AP_EXPORT string8 os_binary_path(void);
AP_EXPORT string8 os_current_path(void);

AP_EXPORT void* os_mem_reserve(u64 size);
AP_EXPORT b32   os_mem_commit(void* ptr, u64 size);
AP_EXPORT void  os_mem_decommit(void* ptr, u64 size);
AP_EXPORT void  os_mem_release(void* ptr, u64 size);

AP_EXPORT u64 os_mem_pagesize(void);

// TODO: make sure linux and windows timestamps are consistent
AP_EXPORT datetime os_now_localtime(void);

AP_EXPORT u64  os_now_microseconds(void);
AP_EXPORT void os_sleep_milliseconds(u32 t);

AP_EXPORT string8    os_file_read(marena* arena, string8 path);
AP_EXPORT b32        os_file_write(string8 path, string8_list str_list);
AP_EXPORT b32        os_file_append(string8 path, string8_list str_list);
AP_EXPORT file_stats os_file_get_stats(string8 path);

AP_EXPORT os_file os_file_open(string8 path, file_mode open_mode);
AP_EXPORT b32     os_file_write_open(os_file file, string8 str);
AP_EXPORT void    os_file_close(os_file file);

AP_EXPORT os_library os_lib_load(string8 path);
AP_EXPORT void_func*  os_lib_func(os_library lib, const char* func_name);
AP_EXPORT void       os_lib_release(os_library lib);

#ifdef __cplusplus
}
#endif

#endif // OS_H
