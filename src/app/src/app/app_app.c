#include "app/app_app.h"

#include "os/os.h"

ap_app* app_create(marena* arena, apres* pres, u32 win_width, u32 win_height) {
    ap_app* app = CREATE_STRUCT(arena, ap_app);

    gfx_window* win = gfx_win_create(arena, win_width, win_height, STR8_LIT("Animated Presentation"));
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    app->win = win;
    app->rectb = draw_rectb_create(arena, win, 1024, 32);
    app->cbezier = draw_cbezier_create(arena, win, 1024),
    app->poly = draw_poly_create(arena, win, 256),
    app->pres = NULL;
    
    return app;
} 

typedef void (plugin_init_func)(marena* arena, obj_register* obj_reg);

void app_run(marena* arena, ap_app* app) {
    draw_rectb_finalize_textures(app->rectb);

    gfx_win_alpha_blend(app->win, true);
    gfx_win_clear_color(app->win, (vec3){ 0.5f, 0.6f, 0.7f });

    obj_register* obj_reg = obj_reg_create(arena, 32);

    os_library test_plugin = os_lib_load(STR8_LIT("./bin/Debug-linux-x86_64/libbuiltin_plugin.so"));
    plugin_init_func* test_init = (plugin_init_func*)os_lib_func(test_plugin, "plugin_init");
    test_init(arena, obj_reg);

    obj_pool* pool = obj_pool_create(arena, obj_reg, 64);

    obj_pool_add(pool, obj_reg, STR8_LIT("rectangle"));

    u64 time_prev = os_now_microseconds();
    while (!app->win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        obj_pool_update(pool, obj_reg, delta);

        gfx_win_clear(app->win);
        
        obj_pool_draw(pool, obj_reg, app);

        draw_rectb_flush(app->rectb);
        draw_cbezier_flush(app->cbezier);

        gfx_win_swap_buffers(app->win);
        gfx_win_process_events(app->win);

        time_prev = time_now;
        u32 sleep_time_ms = MAX(0, 16 - (i64)(delta * 1000));
        os_sleep_milliseconds(sleep_time_ms);
    }
    
    obj_pool_destroy(pool, obj_reg);
    obj_reg_destroy(obj_reg);
    
    os_lib_release(test_plugin);
}

void app_destroy(ap_app* app) {
    draw_rectb_destroy(app->rectb);
    draw_cbezier_destroy(app->cbezier);
    draw_poly_destroy(app->poly);

    gfx_win_destroy(app->win);
}
