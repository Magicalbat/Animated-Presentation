#ifdef AP_PLATFORM_LINUX

#include <stdio.h>
#include <stddef.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "os.h"

arena_t*       lnx_arena;
string8_list_t lnx_cmd_args;

void os_main_init(int argc, char** argv) {
    lnx_arena = arena_create(KB(4));

    for (i32 i = 0; i < argc; i++) {
        string8_t str = str8_from_cstr((u8*)argv[i]);
        str8_list_push(lnx_arena, &lnx_cmd_args, str);
    }
}
void os_main_quit() {
    arena_free(lnx_arena);
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

u64 os_now_microseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}
void os_sleep_milliseconds(u32 t) {
    usleep(t * 1000);
}

// TODO: make sure memory is working correctly for path names
string8_t os_file_read(arena_t* arena, string8_t path) {
    string8_t out = { 0 };
    
    u8* path_cstr = (u8*)arena_alloc(lnx_arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';
    
    struct stat file_stats;

    int fd = open((char*)path_cstr, O_RDONLY);
    fstat(fd, &file_stats);

    arena_pop(lnx_arena, sizeof(u8) * (path.size + 1));
    
    if (S_ISREG(file_stats.st_mode)) {
        out.size = file_stats.st_size;
        out.str = (u8*)arena_alloc(arena, file_stats.st_size);

        int ret = read(fd, out.str, file_stats.st_size);
        // TODO: error checking
    }
    close(fd);

    return out;
}

void os_file_write(string8_t path, string8_list_t str_list) {
    u8* path_cstr = (u8*)arena_alloc(lnx_arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';

    int fd = open((char*)path_cstr, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        char err[256];
        strerror_r(errno, &err[0], 256);
        printf("Could not open file: %s\n", err);
    }

    u64 offset = 0;
    for (string8_node_t* node = str_list.first; node != NULL; node = node->next) {
        int written = pwrite(fd, node->str.str, node->str.size, offset);

        if (written == -1) {
            ASSERT(false, "You forgot to do error handling here :(");
        }

        offset += written;
    }
    close(fd);
}
file_stats_t os_file_get_stats(string8_t path) {
    u8* path_cstr = (u8*)arena_alloc(lnx_arena, sizeof(u8) * (path.size + 1));
    memcpy(path_cstr, path.str, path.size);
    path_cstr[path.size] = '\0';

    struct stat file_stats;
    stat((char*)path_cstr, &file_stats);

    arena_pop(lnx_arena, sizeof(u8) * (path.size + 1));

    return (file_stats_t){
        .size = file_stats.st_size,
        .flags = S_ISDIR(file_stats.st_mode)
        // TODO: above line is dumb
    };
}

#endif 
