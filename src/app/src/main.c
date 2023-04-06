#include "ap_core.h"

#include "app/app.h"

#define WIN_SCALE 3
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

// TODO: system to get presentation file

int main(int argc, char** argv) {
    log_init(&(log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });

    os_main_init(argc, argv);

    marena* perm_arena = marena_create(&(marena_desc){
        .desired_max_size = MiB(4),
        .desired_block_size = KiB(256)
    });

    app_app* app = app_create(perm_arena, STR8_LIT("test.pres"), WIDTH, HEIGHT);

    app_run(app);
    app_destroy(app);

    marena_destroy(perm_arena);
    log_quit();

    return 0;
}
