#ifndef OS_H
#define OS_H

#include "base/base_def.h"

// These functions will be implemented is specific os c files

void* os_mem_reserve  ( uint64_t size            );
void  os_mem_commit   ( void* ptr, uint64_t size );
void  os_mem_decommit ( void* ptr, uint64_t size );
void  os_mem_release  ( void* ptr, uint64_t size );

uint64_t os_mem_pagesize();

#endif