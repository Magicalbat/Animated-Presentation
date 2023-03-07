#ifndef BASE_LOG_H
#define BASE_LOG_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base_defs.h"
#include "base_str.h"
#include "base_mem.h"

// This is to make the log_desc work (see log_init)
#define LOG_NO  1
#define LOG_YES 2

typedef enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARN,
    LOG_ERROR,
    LOG_LEVEL_COUNT
} log_level;

typedef struct {
    string8 str;
    log_level level;
} log_data;

typedef struct {
    b8 log_time;
    b8 new_file;

    string8 file_path;

    u32 max_stored;

    b8 log_stdout[LOG_LEVEL_COUNT];
    b8 log_file[LOG_LEVEL_COUNT];

    u32 colors[LOG_LEVEL_COUNT];
} log_desc;

// log_desc should be made with designated initializers
// the functions checks for zero values to apply defaults
AP_EXPORT void log_init(log_desc* desc);
AP_EXPORT void log_quit(void);
AP_EXPORT void log_msg(log_level level, const char* msg);
AP_EXPORT void log_msgf(log_level level, const char* fmt, ...);
AP_EXPORT log_data log_get_last(log_level level);
AP_EXPORT void log_set_col(log_level level, u32 col);

#define log_info(msg)        log_msg (LOG_INFO, msg)
#define log_infof(fmt, ...)  log_msgf(LOG_INFO, fmt, __VA_ARGS__)
#define log_debug(msg)       log_msg (LOG_DEBUG, msg)
#define log_debugf(fmt, ...) log_msgf(LOG_DEBUG, fmt, __VA_ARGS__)
#define log_warn(msg)        log_msg (LOG_WARN, msg)
#define log_warnf(fmt, ...)  log_msgf(LOG_WARN, fmt, __VA_ARGS__)
#define log_error(msg)       log_msg (LOG_ERROR, msg)
#define log_errorf(fmt, ...) log_msgf(LOG_ERROR, fmt, __VA_ARGS__)

#define log_get_last_info()  log_get_last(LOG_INFO)
#define log_get_last_debug() log_get_last(LOG_DEBUG)
#define log_get_last_warn()  log_get_last(LOG_WARN)
#define log_get_last_error() log_get_last(LOG_ERROR)

#define log_set_info_col(col)  log_set_col(LOG_INFO, col)
#define log_set_debug_col(col) log_set_col(LOG_DEBUG, col)
#define log_set_warn_col(col)  log_set_col(LOG_WARN, col)
#define log_set_error_col(col) log_set_col(LOG_ERROR, col)

#define ANSI_BOLD   1
#define ANSI_ITALIC 3
#define ANSI_UNDER  4
#define ANSI_STRIKE 9

#define ANSI_FG_BLACK   30
#define ANSI_FG_RED     31
#define ANSI_FG_GREEN   32
#define ANSI_FG_YELLOW  33
#define ANSI_FG_BLUE    34
#define ANSI_FG_MAGENTA 35
#define ANSI_FG_CYAN    36
#define ANSI_FG_WHITE   37

#define ANSI_BG_BLACK   40
#define ANSI_BG_RED     41
#define ANSI_BG_GREEN   42
#define ANSI_BG_YELLOW  43
#define ANSI_BG_BLUE    44
#define ANSI_BG_MAGENTA 45
#define ANSI_BG_CYAN    46
#define ANSI_BG_WHITE   47

#define ANSI_FG_B_BLACK   90
#define ANSI_FG_B_RED     91
#define ANSI_FG_B_GREEN   92
#define ANSI_FG_B_YELLOW  93
#define ANSI_FG_B_BLUE    94
#define ANSI_FG_B_MAGENTA 95
#define ANSI_FG_B_CYAN    96
#define ANSI_FG_B_WHITE   97

#define ANSI_BG_B_BLACK   100
#define ANSI_BG_B_RED     101
#define ANSI_BG_B_GREEN   102
#define ANSI_BG_B_YELLOW  103
#define ANSI_BG_B_BLUE    104
#define ANSI_BG_B_MAGENTA 105
#define ANSI_BG_B_CYAN    106
#define ANSI_BG_B_WHITE   107

#ifdef __cplusplus
}
#endif

#endif // BASE_LOG_H
