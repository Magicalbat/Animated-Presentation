#ifdef __EMSCRIPTEN__

#include <stdio.h>
#include <stddef.h>

#include "os/os.h"

static marena*      wasm_arena = NULL;
static string8_list wasm_cmd_args = { };
static string8 wasm_empty_str = { .size = 0, .str = (u8*)"" };

void os_main_init(int argc, char** argv) {
    wasm_arena = marena_create(&(marena_desc){
        .desired_max_size = KiB(16)
    });

    for (i32 i = 0; i < argc; i++) {
        string8 str = str8_from_cstr((u8*)argv[i]);
        str8_list_push(wasm_arena, &wasm_cmd_args, str);
    }
}
void os_main_quit(void) {
    marena_destroy(wasm_arena);
}
string8_list os_get_cmd_args(void) {
    return wasm_cmd_args;
}

string8 os_binary_path(void) {
    return wasm_empty_str;
}
string8 os_current_path(void) { 
    return wasm_empty_str;
}

void* os_mem_reserve(u64 size) {
    void* out = mmap(NULL, size, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    return out;
}
b32 os_mem_commit(void* ptr, u64 size) {
    b32 out = (mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0);
    return out;
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

/*EM_ASYNC_JS(u8*, os_file_read_impl, (char* file_name), {
    const response = await fetch(UTF8ToString(file_name));
    if (!response.ok) {
        return 0;
    }

    const blob = await response.blob();
    const data = new Uint8Array(await blob.arrayBuffer());

    const len = data.length;
    const lengthArr = new Uint8Array(8);
    for (let i = 0; i < 8; i++) {
        lengthArr[i] = (len >>> (i * 8)) & 0xff;
    }
    
    const ptr = _malloc(data.length + 8);

    HEAP8.set(lengthArr, ptr);
    HEAP8.set(data, ptr + 8);

    return ptr;
})

string8 os_file_read(marena* arena, string8 path) {
    marena_temp temp = marena_temp_begin(arena);

    u8* path_cstr = str8_to_cstr(temp.arena, path);
    u8* data = (u8*)os_file_read_impl((char*)path_cstr);

    marena_temp_end(temp);

    if (data == NULL) {
        log_errorf("Failed to read file from %.*s", (int)path.size, path.str);
        return (string8){ .size = 0 };
    }

    u64 size = 0;
    for (u32 i = 0; i < 8; i++) {
        size |= data[i] << (i * 8);
    }

    string8 file = {
        .str = data + 8,
        .size = size
    };

    string8 out = str8_copy(arena, file);

    free(data);

    return out;
}*/
string8 os_file_read(marena* arena, string8 path) {
    AP_UNUSED(arena);
    AP_UNUSED(path);

    log_error("OS file read unsupported in wasm side module");
    
    return (string8){ 0 };
}
b32 os_file_write(string8 path, string8_list str_list) {
    AP_UNUSED(path);
    AP_UNUSED(str_list);

    log_error("OS file write unsupported in wasm");

    return false;
}
b32 os_file_append(string8 path, string8_list str_list) {
    AP_UNUSED(path);
    AP_UNUSED(str_list);

    log_error("OS file append unsupported in wasm");

    return false;
}
file_stats os_file_get_stats(string8 path) {
    AP_UNUSED(path);

    log_error("OS file stats unsupported in wasm");

    return (file_stats){ 0 };
}

os_file os_file_open(string8 path, file_mode open_mode) {
    AP_UNUSED(path);
    AP_UNUSED(open_mode);

    log_error("OS file open unsupported in wasm");

    return (os_file){ 0 };
}
b32 os_file_write_open(os_file file, string8 str) {
    AP_UNUSED(file);
    AP_UNUSED(str);

    log_error("OS file write open unsupported in wasm");

    return false;
}
void os_file_close(os_file file) {
    AP_UNUSED(file);

    log_error("OS file close unsupported in wasm");
}


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

/*EM_ASYNC_JS(int, js_load_lib, (char* file_name), {
    const fileName = UTF8ToString(file_name);

    const response = await fetch(fileName);
    if (!response.ok) {
        return 0;
    }

    const blob = await response.blob();
    const data = new Uint8Array(await blob.arrayBuffer());

    const stream = FS.open(fileName, "w+");
    FS.write(stream, data, 0, data.length, 0);
    FS.close(stream);

    return 1;
})

// I know that this is a bit stupid and convoluted,
// but I do not want to think of a better solution right not
os_library os_lib_load(string8 path) {
    marena_temp temp = marena_temp_begin(wasm_arena);
    
    u8* path_cstr = str8_to_cstr(temp.arena, path);
    
    js_load_lib((char*)path_cstr);
    void* handle = dlopen((char*)path_cstr, RTLD_LAZY);

    marena_temp_end(temp);

    if (handle == NULL) {
        log_dl_errorf("Failed to dynamic library \"%.*s\"", (int)path.size, path.str);
    }

    return (os_library){
        .handle = handle
    };
}*/

os_library os_lib_load(string8 path) {
    AP_UNUSED(path);

    log_error("OS lib load unsupported in wasm side module");
    
    return (os_library){ 0 };
}
void_func* os_lib_func(os_library lib, const char* func_name) {
    void_func* func = (void_func*)dlsym(lib.handle, func_name);
    
    if (func == NULL) {
        log_dl_errorf("Failed to load library function \"%s\"", func_name);
    }

    return func;
}
void os_lib_release(os_library lib) {
    dlclose(lib.handle);
}

#endif // __EMSCRIPTEN__
