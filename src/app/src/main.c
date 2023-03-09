#include "ap_core.h"

#include "app/app.h"

#define WIN_SCALE 1
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init(&(log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });
    
    marena* perm_arena = marena_create(&(marena_desc){
        .desired_max_size = MiB(4),
        .desired_block_size = KiB(64)
    });

    ap_app* app = app_create(perm_arena, NULL, WIDTH, HEIGHT);
    app_run(perm_arena, app);
    app_destroy(app);

    marena_destroy(perm_arena);
    log_quit();

    return 0;
}