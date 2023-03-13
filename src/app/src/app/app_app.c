#include "app/app_app.h"

#include "os/os.h"

ap_app* app_create(marena* arena, string8 pres_path, u32 win_width, u32 win_height) {
    ap_app* app = CREATE_STRUCT(arena, ap_app);

    gfx_window* win = gfx_win_create(arena, win_width, win_height, STR8_LIT("Animated Presentation"));
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    app->win = win;
    app->rectb = draw_rectb_create(arena, win, 1024, 32);
    app->cbezier = draw_cbezier_create(arena, win, 1024);
    app->poly = draw_poly_create(arena, win, 256);
    app->pres = pres_parse(arena, app, pres_path);
    
    return app;
} 


void app_run(marena* arena, ap_app* app) {
    obj_pool* test_pool = obj_pool_create(arena, app->pres->obj_reg, 32);

    obj_ref rect_ref = obj_pool_add(test_pool, app->pres->obj_reg, STR8_LIT("rectangle"));

    f64 prop_data = 50.0;
    obj_ref_set(rect_ref, app->pres->obj_reg, STR8_LIT("y"), &prop_data);
    obj_ref_set(rect_ref, app->pres->obj_reg, STR8_LIT("x"), &prop_data);
    prop_data = 25.0;
    obj_ref_set(rect_ref, app->pres->obj_reg, STR8_LIT("w"), &prop_data);
    obj_ref_set(rect_ref, app->pres->obj_reg, STR8_LIT("h"), &prop_data);

    draw_rectb_finalize_textures(app->rectb);

    gfx_win_alpha_blend(app->win, true);
    gfx_win_clear_color(app->win, (vec3){ 0.5f, 0.6f, 0.7f });

    u64 time_prev = os_now_microseconds();
    while (!app->win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        pres_update(app->pres, delta);
        obj_pool_update(test_pool, app->pres->obj_reg, delta);

        gfx_win_clear(app->win);

        pres_draw(app->pres, app);
        obj_pool_draw(test_pool, app->pres->obj_reg, app);

        draw_rectb_flush(app->rectb);
        draw_cbezier_flush(app->cbezier);

        gfx_win_swap_buffers(app->win);
        gfx_win_process_events(app->win);

        time_prev = time_now;
        u32 sleep_time_ms = MAX(0, 16 - (i64)(delta * 1000));
        os_sleep_milliseconds(sleep_time_ms);
    }

    obj_pool_destroy(test_pool, app->pres->obj_reg);
}

void app_destroy(ap_app* app) {
    pres_delete(app->pres);

    draw_rectb_destroy(app->rectb);
    draw_cbezier_destroy(app->cbezier);
    draw_poly_destroy(app->poly);

    gfx_win_destroy(app->win);
}
