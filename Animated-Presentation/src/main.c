#include <stdio.h>

#include "base/base.h" #include "os.h"

int main() {
    printf("program init\n");

    getwchar();

    uint8_t* buff = os_mem_reserve(4096 * 1000);

    printf("mem reserved\n");
    getwchar();

    os_mem_commit(buff, 4096 * 500);

    printf("mem commited\n");
    getwchar();

    os_mem_decommit(buff, 4096 * 500);

    printf("mem decommited\n");
    getwchar();

    os_mem_release(buff, 4096 * 1000);

    printf("mem released\n");
    getwchar();
	
	return 0;
}
