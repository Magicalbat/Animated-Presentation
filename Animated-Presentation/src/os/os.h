#ifndef OS_H
#define OS_H

#include "base/base_def.h"
#include "base/base_str.h"

// These functions will be implemented is specific os c files

void           os_main_init(int argc, char** argv);
void           os_main_end();
string8_list_t os_get_cmd_args();

void* os_mem_reserve(u64 size);
void  os_mem_commit(void* ptr, u64 size);
void  os_mem_decommit(void* ptr, u64 size);
void  os_mem_release(void* ptr, u64 size);

u64 os_mem_pagesize();

// Load file
// Write to file
// Append to file?
// Load module
// Get datetime
// Functions for profiling?

#endif // OS_H