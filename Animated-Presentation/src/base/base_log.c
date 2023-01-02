#include <stdio.h>
#include <stdarg.h>

#include "base_log.h"

#include "os/os.h"

// TODO: log time, log to file

static string8_t log_names[LOG_LEVEL_COUNT] = {
    { (u8*)"Info",  4 },
    { (u8*)"Debug", 5 },
    { (u8*)"Warn",  4 },
    { (u8*)"Error", 5 },
};

static arena_t* log_arena;
static u64 log_arena_start_pos;

static b32 make_file;
static file_handle_t file;

static log_desc_t log_desc;
static log_msg_t* logs;
static u32 log_index;
static log_msg_t last_logs[LOG_LEVEL_COUNT];

#define TRY_PROP(prop, val) log_desc.prop = desc.prop ? desc.prop : val;
void log_init(log_desc_t desc) {
    log_desc = desc;

    TRY_PROP(max_stored, 256);

    TRY_PROP(colors[LOG_INFO],  ANSI_FG_B_BLACK);
    TRY_PROP(colors[LOG_DEBUG], ANSI_FG_CYAN);
    TRY_PROP(colors[LOG_WARN],  ANSI_FG_YELLOW);
    TRY_PROP(colors[LOG_ERROR], ANSI_FG_RED);

    TRY_PROP(log_stdout[LOG_INFO],  LOG_YES);
    TRY_PROP(log_stdout[LOG_DEBUG], LOG_YES);
    TRY_PROP(log_stdout[LOG_WARN],  LOG_YES);
    TRY_PROP(log_stdout[LOG_ERROR], LOG_YES);

    TRY_PROP(log_file[LOG_INFO],  LOG_NO);
    TRY_PROP(log_file[LOG_DEBUG], LOG_NO);
    TRY_PROP(log_file[LOG_WARN],  LOG_YES);
    TRY_PROP(log_file[LOG_ERROR], LOG_YES);

    log_desc.file_path = desc.file_path;
    if (log_desc.file_path.size == 0) {
        log_desc.file_path = STR8_LIT("log.txt");
    }

    make_file = false;
    for (u32 i = 0; i < LOG_LEVEL_COUNT; i++) {
        if (log_desc.log_file[i] == LOG_YES) {
            make_file = true;
            break;
        }
    }
    if (make_file) {
        file = os_file_open(log_desc.file_path, FOPEN_WRITE);
    }

    log_arena = arena_create(log_desc.max_stored * 256 + KiB(4));

    logs = CREATE_ZERO_ARRAY(log_arena, logs, log_msg_t, log_desc.max_stored);
    log_index = 0;

    for (u32 i = 0; i < LOG_LEVEL_COUNT; i++) {
        last_logs[i] = (log_msg_t){ 0 };
    }

    log_arena_start_pos = log_arena->cur;
}

void log_to_file(log_level_t level, log_msg_t log_msg) {
    os_file_write_open(file, log_names[level]);
    os_file_write_open(file, STR8_LIT(": "));
    os_file_write_open(file, log_msg.msg);
    os_file_write_open(file, STR8_LIT("\n"));
}

void log_msg(log_level_t level, const char* msg) {
    if (log_index + 1 >= log_desc.max_stored) {
        arena_pop_to(log_arena, log_arena_start_pos);
        log_index = 0;
    }

    log_msg_t log_msg = (log_msg_t){
        .msg = str8_from_cstr((u8*)msg),
        .level = level
    };
    logs[log_index++] = log_msg;
    last_logs[level]  = log_msg;

    if (log_desc.log_stdout[level] == LOG_YES) {
        fprintf(stdout, "\033[%um%s: ", log_desc.colors[level], log_names[level].str);
        fputs(msg, stdout);
        fputs("\033[m\n", stdout);
    }
    if (log_desc.log_file[level] == LOG_YES) {
        log_to_file(level, log_msg);
    }
}
void log_quit() {
    arena_destroy(log_arena);

    if (make_file) {
        os_file_close(file);
    }
}
void log_msgf(log_level_t level, const char* fmt, ...) {
    if (log_index + 1 >= log_desc.max_stored) {
        arena_pop_to(log_arena, log_arena_start_pos);
        log_index = 0;
    }

    va_list args;
    va_start(args, fmt);

    string8_t msg_str = str8_pushfv(log_arena, fmt, args);

    va_end(args);

    log_msg_t log_msg = (log_msg_t){
        .msg = msg_str,
        .level = level
    };

    logs[log_index++] = log_msg;
    last_logs[level]  = log_msg;

    if (log_desc.log_stdout[level] == LOG_YES) {
        fprintf(stdout, "\033[%um%s: ", log_desc.colors[level], log_names[level].str);
        fprintf(stdout, "%.*s", (int)msg_str.size, (char*)msg_str.str);
        fputs("\033[m\n", stdout);
    }
    if (log_desc.log_file[level] == LOG_YES) {
        log_to_file(level, log_msg);
    }
}
log_msg_t log_get_last(log_level_t level) { 
    return last_logs[level];
}
void log_set_col(log_level_t level, u32 col) {
    log_desc.colors[level] = col;
}
