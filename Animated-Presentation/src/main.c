#include "os/os.h"
#include "base/base.h"

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    string8_list_t args = os_get_cmd_args();
    for (string8_node_t* node = args.first; node != NULL; node = node->next) {
        printf("%.*s\n", (i32)node->str.size, node->str.str);
    }
    
    os_main_end();
	return 0;
}
