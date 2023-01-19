#ifdef __linux__

#include <stdio.h>
#include <stddef.h>

#include "os.h"

static arena_t*       lnx_arena;
static string8_list_t lnx_cmd_args;

static string8_t lnx_error_string() {
    char* err_cstr = strerror(errno);
    
    return str8_from_cstr(err_cstr);
}
#define log_lnx_error(msg) do { \
        string8_t err = lnx_error_string(); \
        log_errorf(msg ", Linux Error: %.*s", (int)err.size, err.str); \
    } while (0)
#define log_lnx_errorf(fmt, ...) do { \
        string8_t err = lnx_error_string(); \
        log_errorf(fmt ", Linux Error: %.*s", __VA_ARGS__, (int)err.size, err.str); \
    } while (0)


void os_main_init(int argc, char** argv) {
    lnx_arena = arena_create(KiB(16));

    for (i32 i = 0; i < argc; i++) {
        string8_t str = str8_from_cstr((u8*)argv[i]);
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

uint64_t os_mem_pagesize() {
    return sysconf(_SC_PAGE_SIZE);
}

datetime_t os_now_localtime() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    return (datetime_t){
        .sec = tm.tm_sec,
        .min = tm.tm_min,
        .hour = tm.tm_hour,
        .day = tm.tm_mday,
        .month = tm.tm_mon + 1,
        .year = tm.tm_year + 1900
    };
}

u64 os_now_microseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
void os_sleep_milliseconds(u32 t) {
    usleep(t * 1000);
}

// TODO: str8_to_cstr function
int lnx_open_impl(string8_t path, int flags, mode_t mode) {
    arena_temp_t temp = arena_temp_begin(lnx_arena);
    
    u8* path_cstr = (u8*)arena_alloc(temp.arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';
    
    int fd = open((char*)path_cstr, flags, mode);

    arena_temp_end(temp);

    return fd;
}

string8_t os_file_read(arena_t* arena, string8_t path) {
    int fd = lnx_open_impl(path, O_RDONLY, 0);
    
    if (fd == -1) {
        log_lnx_errorf("Failed to open file \"%.*s\"", (int)path.size, path.str);

        return (string8_t){ 0 };
    }
    
    struct stat file_stats;
    fstat(fd, &file_stats);

    string8_t out = { 0 };

    if (S_ISREG(file_stats.st_mode)) {
        out.size = file_stats.st_size;
        out.str = (u8*)arena_alloc(arena, (u64)file_stats.st_size);

        if (read(fd, out.str, file_stats.st_size) == -1) {
            log_lnx_errorf("Failed to read file \"%.*s\"", (int)path.size, path.str);
            
            close(fd);
            
            return (string8_t){ 0 };
        }
    } else {
        log_errorf("Failed to read file \"%.*s\", file is not regular", (int)path.size, path.str);
    }
    close(fd);

    return out;
}

b32 os_file_write(string8_t path, string8_list_t str_list) {
    int fd = lnx_open_impl(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        log_lnx_errorf("Failed to open file \"%.*s\"", (int)path.size, path.str);

        return false;
    }

    b32 out = true;
    
    for (string8_node_t* node = str_list.first; node != NULL; node = node->next) {
        ssize_t written = write(fd, node->str.str, node->str.size);

        if (written == -1) {
            log_lnx_errorf("Failed to write to file \"%.*s\"", (int)path.size, path.str);

            out = false;
            break;
        }
    }
        
    close(fd);

    return out;
}
b32 os_file_append(string8_t path, string8_list_t str_list) {
    int fd = lnx_open_impl(path, O_APPEND | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        log_lnx_errorf("Failed to open file \"%.*s\"", (int)path.size, path.str);

        return false;
    }
    
    b32 out = true;

    for (string8_node_t* node = str_list.first; node != NULL; node = node->next) {
        ssize_t written = write(fd, node->str.str, node->str.size);
        
        if (written == -1) {
            log_lnx_errorf("Failed to append to file \"%.*s\"", (int)path.size, path.str);

            out = false;
            break;
        }
    }

    close(fd);
    
    return out;
}
file_flags_t lnx_file_flags(mode_t mode) {
    file_flags_t flags;

    if (S_ISDIR(mode))
        flags |= FILE_IS_DIR;

    return flags;
}
file_stats_t os_file_get_stats(string8_t path) {
    arena_temp_t temp = arena_temp_begin(lnx_arena);
    
    u8* path_cstr = (u8*)arena_alloc(temp.arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';

    struct stat file_stats;
    stat((char*)path_cstr, &file_stats);

    arena_temp_end(temp);

    return (file_stats_t){
        .size = file_stats.st_size,
        .flags = lnx_file_flags(file_stats.st_mode)
    };
}

os_file_t os_file_open(string8_t path, file_mode_t open_mode) {
    int fd = -1;

    switch (open_mode) {
        case FOPEN_READ:
            fd = lnx_open_impl(path, O_RDONLY, 0);
            break;
        case FOPEN_WRITE:
            fd = lnx_open_impl(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
            break;
        case FOPEN_APPEND:
            fd = lnx_open_impl(path, O_APPEND | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
            break;
        default: break;
    }

    if (fd == -1) {
        log_lnx_errorf("Failed to open file \"%.*s\"", (int)path.size, (char*)path.str);
    }
    
    return (os_file_t) { .fd = fd };
}
b32 os_file_write_open(os_file_t file, string8_t str) {
    ssize_t written = write(file.fd, str.str, str.size);

    if (written == -1) {
        log_lnx_error("Failed to write to open file");
        
        return false;
    }
    
    return true;
}
void os_file_close(os_file_t file) {
    close(file.fd);
}

static string8_t dl_error_string() {
    char* err_cstr = dlerror();
    
    return str8_from_cstr(err_cstr);
}
#define log_dl_error(msg) do { \
        string8_t err = dl_error_string(); \
        log_errorf(msg ", Linux DL Error: %.*s", (int)err.size, err.str); \
    } while (0)
#define log_dl_errorf(fmt, ...) do { \
        string8_t err = dl_error_string(); \
        log_errorf(fmt ", Linux DL Error: %.*s", __VA_ARGS__, (int)err.size, err.str); \
    } while (0)

os_library_t os_lib_load(string8_t path) {
    arena_temp_t temp = arena_temp_begin(lnx_arena);
    
    u8* path_cstr = (u8*)arena_alloc(temp.arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';
    
    void* handle = dlopen((char*)path_cstr, RTLD_LAZY);

    arena_temp_end(temp);

    if (handle == NULL) {
        log_dl_errorf("Failed to dynamic library \"%.*s\"", (int)path.size, path.str);
    }

    return (os_library_t){
        .handle = handle
    };

}
void_func_t os_lib_func(os_library_t lib, const char* func_name) {
    void_func_t func = (void_func_t)dlsym(lib.handle, func_name);
    
    if (func == NULL) {
        log_dl_errorf("Failed to load library function \"%s\"", func_name);
    }

    return func;
}
void os_lib_release(os_library_t lib) {
    dlclose(lib.handle);
}

#endif // __linux__
