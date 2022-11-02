#include <stdio.h>

#include "base/base.h"
#include "os.h"

int main() {
    printf("Page Size: %lu\n", (unsigned long)os_mem_pagesize());
    
    printf("program init\n");

    uint64_t size = MB(1024);
    uint8_t* buff = os_mem_reserve(size);

    printf("mem reserved\n");

    os_mem_commit(buff, size >> 1);

    printf("mem commited\n");

    os_mem_decommit(buff, size >> 1);

    printf("mem decommited\n");

    os_mem_release(buff, size);

    printf("mem released\n");

	return 0;
}
