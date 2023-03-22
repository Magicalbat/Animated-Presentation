#include "ap_core.h"

#include "app/app.h"

#define WIN_SCALE 1
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

#ifdef __EMSCRIPTEN__

EM_JS(void, test, (), {
    const canvas = document.querySelector("#canvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});

#endif


int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init(&(log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });
    
    marena* perm_arena = marena_create(&(marena_desc){
        .desired_max_size = MiB(4),
        .desired_block_size = KiB(256)
    });

    #ifdef __EMSCRIPTEN__

    test();

    #endif

    
    app_app* app = app_create(perm_arena, STR8_LIT("test.pres"), WIDTH, HEIGHT);
    
    app_run(perm_arena, app);
    app_destroy(app);

    marena_destroy(perm_arena);
    log_quit();

    return 0;
}
