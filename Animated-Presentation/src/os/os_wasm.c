#ifdef __EMSCRIPTEN__

#include <stdio.h>
#include <stddef.h>

#include "os.h"

static arena*       wasm_arena;
static string8_list wasm_cmd_args;

void os_main_init(int argc, char** argv) {
    wasm_arena = arena_create(KiB(16));

    for (i32 i = 0; i < argc; i++) {
        string8 str = str8_from_cstr((u8*)argv[i]);
        str8_list_push(wasm_arena, &wasm_cmd_args, str);
    }
}
void os_main_quit(void) {
    arena_destroy(wasm_arena);
}
string8_list os_get_cmd_args(void) {
    return wasm_cmd_args;
}

void* os_mem_reserve(u64 size) {
    void* out = mmap(NULL, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    return out;
}
void os_mem_commit(void* ptr, u64 size) {
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}
void os_mem_decommit(void* ptr, u64 size) {
    mprotect(ptr, size, PROT_NONE);
    madvise(ptr, size, MADV_DONTNEED);
}
void os_mem_release(void* ptr, u64 size) {
    munmap(ptr, size);
}

u64 os_mem_pagesize(void) {
    return sysconf(_SC_PAGE_SIZE);
}

datetime os_now_localtime(void) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    return (datetime){
        .sec = tm.tm_sec,
        .min = tm.tm_min,
        .hour = tm.tm_hour,
        .day = tm.tm_mday,
        .month = tm.tm_mon + 1,
        .year = tm.tm_year + 1900
    };
}

u64 os_now_microseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

void os_sleep_milliseconds(u32 t) {
    emscripten_sleep(t);
}

// TODO: make this work better
// https://stackoverflow.com/questions/63959571/how-do-i-pass-a-file-blob-from-javascript-to-emscripten-webassembly-c
EM_ASYNC_JS(char*, os_file_read_impl, (char* file_name), {
    const response = await fetch(UTF8ToString(file_name));
    if (!response.ok) {
        return 0;
    }

    const text = await response.text();

    const lengthBytes = lengthBytesUTF8(text)+1;
    const stringOnWasmHeap = _malloc(lengthBytes);
    stringToUTF8(text, stringOnWasmHeap, lengthBytes);

    return stringOnWasmHeap;
})

string8 os_file_read(arena* arena, string8 path) {
    arena_temp temp = arena_temp_begin(arena);

    u8* path_cstr = str8_to_cstr(temp.arena, path);
    u8* file = (u8*)os_file_read_impl((char*)path_cstr);

    arena_temp_end(temp);

    string8 out = { 0 }; 
    if (file != NULL) {
        out = str8_copy(arena, str8_from_cstr(file));
    } else {
        log_errorf("Failed to read file from %.*s", (int)path.size, path.str);
    }

    free(file);

    return out;
}
b32 os_file_write(string8 path, string8_list str_list) {
    log_error("OS file write unsupported in wasm");

    return false;
}
b32 os_file_append(string8 path, string8_list str_lit) {
    log_error("OS file append unsupported in wasm");

    return false;
}
file_stats os_file_get_stats(string8 path) {
    log_error("OS file stats unsupported in wasm");

    return (file_stats){ 0 };
}

os_file os_file_open(string8 path, file_mode open_mode) {
    log_error("OS file open unsupported in wasm");

    return (os_file){ 0 };
}
b32 os_file_write_open(os_file file, string8 str) {
    log_error("OS file write open unsupported in wasm");

    return false;
}
void os_file_close(os_file file) {
    log_error("OS file close unsupported in wasm");
}


// TODO: make this work on wasm
static string8 dl_error_string(void) {
    char* err_cstr = dlerror();
    
    return str8_from_cstr((u8*)err_cstr);
}
#define log_dl_error(msg) do { \
        string8 err = dl_error_string(); \
        log_errorf(msg ", Linux DL Error: %.*s", (int)err.size, err.str); \
    } while (0)
#define log_dl_errorf(fmt, ...) do { \
        string8 err = dl_error_string(); \
        log_errorf(fmt ", Linux DL Error: %.*s", __VA_ARGS__, (int)err.size, err.str); \
    } while (0)

os_library os_lib_load(string8 path) {
    arena_temp temp = arena_temp_begin(wasm_arena);
    
    u8* path_cstr = str8_to_cstr(temp.arena, path);
    
    void* handle = dlopen((char*)path_cstr, RTLD_LAZY);

    arena_temp_end(temp);

    if (handle == NULL) {
        log_dl_errorf("Failed to dynamic library \"%.*s\"", (int)path.size, path.str);
    }

    return (os_library){
        .handle = handle
    };

}
void_func os_lib_func(os_library lib, const char* func_name) {
    void_func func = (void_func)dlsym(lib.handle, func_name);
    
    if (func == NULL) {
        log_dl_errorf("Failed to load library function \"%s\"", func_name);
    }

    return func;
}
void os_lib_release(os_library lib) {
    dlclose(lib.handle);
}

#endif // __EMSCRIPTEN__
