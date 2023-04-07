#include "ap_core.h"

#include "app/app.h"

#define WIN_SCALE 3
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

#ifdef __EMSCRIPTEN__
EM_JS(u8*, js_get_pres_source, (), {
    const source = document.getElementById("presSource").innerHTML;

    const ptr = _malloc(source.length + 1);
    stringToUTF8(source, ptr, source.length + 1);

    return ptr;
})
#endif

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

    string8 pres_source = STR8_LIT("main.pres"); 

    #ifdef __EMSCRIPTEN__

    u8* cstr_source = js_get_pres_source();
    pres_source = str8_copy(perm_arena, str8_from_cstr(cstr_source));
    free(cstr_source);

    #else

    // I know that this is not using the os args,
    // but it is in the main function anyways, and this is easier
    if (argc > 1) {
        pres_source = str8_from_cstr((u8*)argv[1]);
    }

    #endif

    app_app* app = app_create(perm_arena, pres_source, WIDTH, HEIGHT);

    app_run(app);
    app_destroy(app);

    marena_destroy(perm_arena);
    log_quit();

    return 0;
}
