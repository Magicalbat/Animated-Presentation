#ifdef AP_PLATFORM_WINDOWS

#include <Windows.h>

#include "base/base.h"
#include "os.h"

static u64            w32_ticks_per_second;

static arena_t*       w32_arena;
static string8_list_t w32_cmd_args;

void os_main_init(int argc, char** argv) {
    LARGE_INTEGER perf_freq;
    if (QueryPerformanceFrequency(&perf_freq)) {
        w32_ticks_per_second = ((u64)perf_freq.HighPart << 32) | perf_freq.LowPart;
    }
    timeBeginPeriod(1);

    w32_arena = arena_create(KB(4));

    for (i32 i = 0; i < argc; i++) {
        string8_t str = str8_from_cstr((u8*)argv[i]);
        str8_list_push(w32_arena, &w32_cmd_args, str);
    }
}
void os_main_quit() {
    arena_destroy(w32_arena);
    timeEndPeriod(1);
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

u64 os_now_microseconds() {
    u64 out = 0;
    LARGE_INTEGER perf_count;
    if (QueryPerformanceCounter(&perf_count)) {
        u64 ticks = ((u64)perf_count.HighPart << 32) | perf_count.LowPart;
        out = ticks * 1000000 / w32_ticks_per_second;
    }
    return out;
}
void os_sleep_milliseconds(u32 t) {
    Sleep(t);
}

// TODO: make sure memory is working correctly for path names
string8_t os_file_read(arena_t* arena, string8_t path) {
    string8_t out = { 0 };

    string16_t path16 = str16_from_str8(w32_arena, path);

    HANDLE file_handle = CreateFile(
        (LPCSTR)path16.str,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED,
        NULL
    );

    arena_pop(w32_arena, sizeof(u16) * path16.size);

    // TODO: is a loop necessary?
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD high_size = 0;
        DWORD low_size = GetFileSize(file_handle, &high_size);
        u64 size_to_read = ((u64)high_size << 32) | low_size;

        printf("%llu\n", size_to_read);
        u8* buffer = arena_alloc(arena, sizeof(u8) * size_to_read);
        DWORD size_read = 0;
        if (ReadFile(file_handle, buffer, size_to_read, &size_read, NULL) != FALSE) {
            u64 to_free = size_to_read - size_read;
            if (to_free > 0) { 
                arena_pop(arena, to_free);
                printf("testing\n");
            }

            out.str = buffer;
            out.size = size_read;
        } else {
            arena_pop(arena, sizeof(u8) * size_to_read);
            // TODO: error stuff here
        }
    }

    CloseHandle(file_handle);
    return out;
}
void os_file_write(string8_t path, string8_list_t str_list) {
    string16_t path16 = str16_from_str8(w32_arena, path);

    HANDLE file_handle = CreateFile(
        (LPCSTR)path16.str,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    arena_pop(w32_arena, sizeof(u16) * path16.size);

    if (file_handle != INVALID_HANDLE_VALUE) {
        for (string8_node_t* node = str_list.first; node != NULL; node = node->next) {
            DWORD to_write = node->str.size;
            // TODO: range checking, i guess

            DWORD written = 0;
            if (WriteFile(file_handle, node->str.str, to_write, &written, 0) == FALSE) {
                // TODO: error checking
            }
        }
    }
    CloseHandle(file_handle);
}
file_stats_t os_file_get_stats(string8_t path) {
    file_stats_t stats = { 0 };

    string16_t path16 = str16_from_str8(w32_arena, path);

    WIN32_FILE_ATTRIBUTE_DATA attribs;
    if (GetFileAttributesEx((LPCSTR)path16.str, GetFileExInfoStandard, &attribs) != FALSE) {
        stats.size = ((u64)attribs.nFileSizeHigh << 32) | attribs.nFileSizeLow;
        if (attribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            stats.flags |= FILE_IS_DIR;
        }
    }

    arena_pop(w32_arena, sizeof(u16) * path16.size);

    return stats;
}

#endif
