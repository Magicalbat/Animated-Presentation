#ifndef OS_H
#define OS_H

#include "base/base_def.h"

// These functions will be implemented is specific os c files

void* os_mem_reserve  ( u64 size            );
void  os_mem_commit   ( void* ptr, u64 size );
void  os_mem_decommit ( void* ptr, u64 size );
void  os_mem_release  ( void* ptr, u64 size );

u64 os_mem_pagesize();

#endif // OS_H