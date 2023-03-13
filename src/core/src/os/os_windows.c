#ifdef _WIN32

#include "base/base.h"
#include "os/os.h"

static u64          w32_ticks_per_second;

static marena*      w32_arena;
static string8_list w32_cmd_args;

static string8 win32_error_string(void) {
    DWORD err = GetLastError();
    if (err == 0) {
        return (string8){ 0 };
    }

    LPSTR msg_buf = NULL;
    DWORD msg_size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPSTR)&msg_buf, // Very intuitive win32
        0, NULL
    );

    string8 out;
    out.size = (u64)msg_size - 3;
    out.str = CREATE_ARRAY(w32_arena, u8, (u64)msg_size - 3);

    memcpy(out.str, msg_buf, msg_size);

    LocalFree(msg_buf);

    return out;
}

#define log_w32_error(msg) do { \
        string8 err = win32_error_string(); \
        log_errorf(msg ", Win32 Error: %.*s", (int)err.size, err.str); \
    } while (0)
#define log_w32_errorf(fmt, ...) do { \
        string8 err = win32_error_string(); \
        log_errorf(fmt ", Win32 Error: %.*s", __VA_ARGS__, (int)err.size, err.str); \
    } while (0)


void os_main_init(int argc, char** argv) {
    w32_arena = marena_create(&(marena_desc){
        .desired_max_size = KiB(16)
    });

    LARGE_INTEGER perf_freq;
    if (QueryPerformanceFrequency(&perf_freq)) {
        w32_ticks_per_second = ((u64)perf_freq.HighPart << 32) | perf_freq.LowPart;
    }
    timeBeginPeriod(1);

    for (i32 i = 0; i < argc; i++) {
        string8 str = str8_from_cstr((u8*)argv[i]);
        str8_list_push(w32_arena, &w32_cmd_args, str);
    }
}
void os_main_quit(void) {
    marena_destroy(w32_arena);
    timeEndPeriod(1);
}
string8_list os_get_cmd_args(void) {
    return w32_cmd_args;
}

void* os_mem_reserve(u64 size) {
    void* out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return out;
}
b32 os_mem_commit(void* ptr, u64 size) {
    b32 out = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
    return out;
}
void os_mem_decommit(void* ptr, u64 size) {
    VirtualFree(ptr, size, MEM_DECOMMIT);
}
void os_mem_release(void* ptr, u64 size) {
    VirtualFree(ptr, 0, MEM_RELEASE);
}

u64 os_mem_pagesize(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

datetime os_now_localtime(void) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    return (datetime){
        .sec   = (u8 )t.wSecond,
        .min   = (u8 )t.wMinute,
        .hour  = (u8 )t.wHour,
        .day   = (u8 )t.wDay,
        .month = (u8 )t.wMonth,
        .year  = (i32)t.wYear + 1601
    };
}

u64 os_now_microseconds(void) {
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

string8 os_file_read(marena* arena, string8 path) {
    marena_temp temp = marena_temp_begin(w32_arena);

    string16 path16 = str16_from_str8(temp.arena, path);

    HANDLE file_handle = CreateFile(
        (LPCWSTR)path16.str,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    marena_temp_end(temp);

    if (file_handle == INVALID_HANDLE_VALUE) {
        log_w32_errorf("Failed to open file \"%.*s\"", (int)path.size, (char*)path.str);

        return (string8){ 0 };
    }

    string8 out = { 0 };

    DWORD high_size = 0;
    DWORD low_size = GetFileSize(file_handle, &high_size);
    u64 total_size = ((u64)high_size << 32) | low_size;

    marena_temp possible_temp = marena_temp_begin(arena);

    u8* buffer = CREATE_ARRAY(arena, u8, total_size);

    u64 total_read = 0;
    while (total_read < total_size) {
        u64 to_read64 = total_size - total_read;
        DWORD to_read = to_read64 > ~(DWORD)(0) ? ~(DWORD)(0) : (DWORD)to_read64;

        DWORD bytes_read = 0;
        if (ReadFile(file_handle, buffer + total_read, to_read, &bytes_read, 0) == FALSE) {
            log_w32_errorf("Failed to read to file \"%.*s\"", (int)path.size, (char*)path.str);
            marena_temp_end(possible_temp);

            return (string8){ 0 };
        }

        total_read += bytes_read;
    }

    out.size = total_size;
    out.str = buffer;

    CloseHandle(file_handle);
    return out;
}

b32 os_file_write_impl(HANDLE file_handle, string8_list str_list) {
    for (string8_node* node = str_list.first; node != NULL; node = node->next) {
        u64 total_to_write = node->str.size;
        u64 total_written = 0;

        while (total_written < total_to_write) {
            u64 to_write64 = total_to_write - total_written;
            DWORD to_write = to_write64 > ~(DWORD)(0) ? ~(DWORD)(0) : (DWORD)to_write64;

            DWORD written = 0;
            if (WriteFile(file_handle, node->str.str + total_written, to_write, &written, 0) == FALSE) {
                return false;
            }

            total_written += written;
        }
    }

    return true;
}

b32 os_file_write(string8 path, string8_list str_list) {
    marena_temp temp = marena_temp_begin(w32_arena);

    string16 path16 = str16_from_str8(temp.arena, path);

    HANDLE file_handle = CreateFile(
        (LPCWSTR)path16.str,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    marena_temp_end(temp);

    if (file_handle == INVALID_HANDLE_VALUE) {
        log_w32_errorf("Failed to open file \"%.*s\"", (int)path.size, (char*)path.str);

        return false;
    }

    b32 out = true;

    if (!os_file_write_impl(file_handle, str_list)) {
        log_w32_errorf("Failed to write to file \"%.*s\"", (int)path.size, (char*)path.str);

        out = false;
    }

    CloseHandle(file_handle);

    return out;
}
b32 os_file_append(string8 path, string8_list str_list) {
    marena_temp temp = marena_temp_begin(w32_arena);

    string16 path16 = str16_from_str8(temp.arena, path);

    HANDLE file_handle = CreateFile(
        (LPCWSTR)path16.str,
        FILE_APPEND_DATA,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    marena_temp_end(temp);

    if (file_handle == INVALID_HANDLE_VALUE) {
        log_w32_errorf("Failed to open file \"%.*s\"", (int)path.size, (char*)path.str);

        return false;
    }

    b32 out = true;

    if (!os_file_write_impl(file_handle, str_list)) {
        log_w32_errorf("Failed to append to file \"%.*s\"", (int)path.size, (char*)path.str);

        out = false;
    }

    CloseHandle(file_handle);
    return out;
}
file_stats os_file_get_stats(string8 path) {
    file_stats stats = { 0 };

    marena_temp temp = marena_temp_begin(w32_arena);

    string16 path16 = str16_from_str8(temp.arena, path);

    WIN32_FILE_ATTRIBUTE_DATA attribs = { 0 };
    if (GetFileAttributesEx((LPCWSTR)path16.str, GetFileExInfoStandard, &attribs) != FALSE) {
        stats.size = ((u64)attribs.nFileSizeHigh << 32) | attribs.nFileSizeLow;
        if (attribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            stats.flags |= FILE_IS_DIR;
        }
    } else {
        log_w32_errorf("Failed to open file \"%.*s\"",  (int)path.size, (char*)path.str);
    }

    marena_temp_end(temp);

    return stats;
}


os_file os_file_open(string8 path, file_mode open_mode) {
    DWORD read_write = 0;
    switch (open_mode) {
        case FOPEN_READ:   read_write = GENERIC_READ;     break;
        case FOPEN_WRITE:  read_write = GENERIC_WRITE;    break;
        case FOPEN_APPEND: read_write = FILE_APPEND_DATA; break;
        default: break;
    }

    DWORD create = 0;
    switch (open_mode){
        case FOPEN_APPEND:
        case FOPEN_READ:  create = OPEN_ALWAYS;   break;
        case FOPEN_WRITE: create = CREATE_ALWAYS; break;
        default: break;
    }

    marena_temp temp = marena_temp_begin(w32_arena);
    
    string16 path16 = str16_from_str8(temp.arena, path);
    
    HANDLE file_handle = CreateFile(
        (LPCWSTR)path16.str,
        read_write,
        0,
        NULL,
        create,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    marena_temp_end(temp);

    if (file_handle == INVALID_HANDLE_VALUE) {
        log_w32_errorf("Failed to open file \"%.*s\"", (int)path.size, (char*)path.str);

        return (os_file) { NULL };
    }

    return (os_file){
        .file_handle = file_handle
    };
}
b32 os_file_write_open(os_file file, string8 str) {
    u64 total_to_write = str.size;
    u64 total_written = 0;

    while (total_written < total_to_write) {
        u64 to_write64 = total_to_write - total_written;
        DWORD to_write = to_write64 > ~(DWORD)(0) ? ~(DWORD)(0) : (DWORD)to_write64;

        DWORD written = 0;
        if (WriteFile(file.file_handle, str.str + total_written, to_write, &written, 0) == FALSE) {
            log_w32_error("Failed to write to open file");

            return false;
        }

        total_written += written;
    }

    return true;
}
void os_file_close(os_file file) {
    CloseHandle(file.file_handle);
}

os_library os_lib_load(string8 path) {
    marena_temp temp = marena_temp_begin(w32_arena);

    string16 path16 = str16_from_str8(temp.arena, path);
    HMODULE module = LoadLibraryW((LPCWSTR)path16.str);

    marena_temp_end(temp);

    if (module == NULL) {
        log_w32_errorf("Failed to dynamic library \"%.*s\"", (int)path.size, path.str);
    }

    return (os_library){
        .module = module
    };
}
void_func os_lib_func(os_library lib, const char* func_name) {
    void_func func = (void_func)GetProcAddress(lib.module, func_name);

    if (func == NULL) {
        log_w32_errorf("Failed to load library function \"%s\"", func_name);
    }

    return func;
}
void os_lib_release(os_library lib) {
    if (FreeLibrary(lib.module) == FALSE) {
        log_w32_error("Failed to free dynamic library");
    }
}

#endif // _WIN32
