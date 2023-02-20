#include <stdio.h>
#include <stdarg.h>

#include "base_log.h"

#include "os/os.h"

static string8 log_names[LOG_LEVEL_COUNT] = {
    { (u8*)"Info",  4 },
    { (u8*)"Debug", 5 },
    { (u8*)"Warn",  4 },
    { (u8*)"Error", 5 },
};

static marena* log_arena;
static u64 log_marena_start_pos;

static b32 make_file;
static os_file file;

static log_desc desc;
static log_data* logs;
static u32 log_index;
static log_data last_logs[LOG_LEVEL_COUNT];

#define TRY_PROP(prop, val) desc.prop = base_desc.prop ? base_desc.prop : val;

void log_init(log_desc base_desc) {
    desc = base_desc;

    TRY_PROP(log_time, LOG_YES);
    TRY_PROP(new_file, LOG_YES);
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

    desc.file_path = desc.file_path;
    if (desc.file_path.size == 0) {
        desc.file_path = STR8_LIT("log.txt");
    }

#ifndef __EMSCRIPTEN__
    make_file = false;
    for (u32 i = 0; i < LOG_LEVEL_COUNT; i++) {
        if (desc.log_file[i] == LOG_YES) {
            make_file = true;
            break;
        }
    }
    if (make_file) {
        file_mode open_mode = desc.new_file == LOG_YES ? FOPEN_WRITE : FOPEN_APPEND;
        file = os_file_open(desc.file_path, open_mode);
    }
#else
    for (u32 i = 0; i < LOG_LEVEL_COUNT; i++) {
        desc.log_file[i] = LOG_NO;
    }
#endif

    log_arena = marena_create(&(marena_desc){
        .desired_max_size = desc.max_stored * 256 + KiB(4)
    });

    logs = CREATE_ZERO_ARRAY(log_arena, log_data, desc.max_stored);
    log_index = 0;

    for (u32 i = 0; i < LOG_LEVEL_COUNT; i++) {
        last_logs[i] = (log_data){ 0 };
    }

    log_marena_start_pos = log_arena->pos;
}
void log_quit(void) {
    marena_destroy(log_arena);

    if (make_file) {
        os_file_close(file);
    }
}

void log_impl(log_data msg) {
    logs[log_index++] = msg;
    last_logs[msg.level]  = msg;

    datetime datetime = os_now_localtime();
    
    if (desc.log_stdout[msg.level] == LOG_YES) {
        if (desc.log_time == LOG_YES) {
            fprintf(stdout, "(%02d:%02d:%02d) ", datetime.hour, datetime.min, datetime.sec);
        }
        fprintf(stdout, "\033[%um%s", desc.colors[msg.level], log_names[msg.level].str);
        fprintf(stdout, ": %.*s\033[m\n", (int)msg.str.size, msg.str.str);
    }
    if (desc.log_file[msg.level] == LOG_YES) {
        if (desc.log_time == LOG_YES) {
            marena_temp temp = marena_temp_begin(log_arena);
            
            string8 time_str = str8_pushf(temp.arena, "(%02d:%02d:%02d) ", datetime.hour, datetime.min, datetime.sec);
            os_file_write_open(file, time_str);
            
            marena_temp_end(temp);
        }
        os_file_write_open(file, log_names[msg.level]);
        os_file_write_open(file, STR8_LIT(": "));
        os_file_write_open(file, msg.str);
        os_file_write_open(file, STR8_LIT("\n"));
    }
}
void log_msg(log_level level, const char* msg_cstr) {
    if (log_index + 1 >= desc.max_stored) {
        marena_pop_to(log_arena, log_marena_start_pos);
        log_index = 0;
    }

    log_data msg = (log_data){
        .str = str8_from_cstr((u8*)msg_cstr),
        .level = level
    };

    log_impl(msg);
}

void log_msgf(log_level level, const char* fmt, ...) {
    if (log_index + 1 >= desc.max_stored) {
        marena_pop_to(log_arena, log_marena_start_pos);
        log_index = 0;
    }

    va_list args;
    va_start(args, fmt);

    string8 msg_str = str8_pushfv(log_arena, fmt, args);

    va_end(args);

    log_data msg = (log_data){
        .str = msg_str,
        .level = level
    };

    log_impl(msg);
}
log_data log_get_last(log_level level) { 
    return last_logs[level];
}
void log_set_col(log_level level, u32 col) {
    desc.colors[level] = col;
}
