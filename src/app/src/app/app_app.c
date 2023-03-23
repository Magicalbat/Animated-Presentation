#include "app/app.h"

#include "os/os.h"

#ifdef __EMSCRIPTEN__
EM_JS(void, app_maximize_canvas, (), {
    const canvas = document.querySelector("#canvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});
#endif


app_app* app_create(marena* arena, string8 pres_path, u32 win_width, u32 win_height) {
    app_app* app = CREATE_STRUCT(arena, app_app);

    gfx_window* win = gfx_win_create(arena, win_width, win_height, STR8_LIT("Animated Presentation"));

#ifdef __EMSCRIPTEN__
    app_maximize_canvas();
#endif
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    app->win = win;
    
    app->bg_col = (vec4d){ .5, .6, .7, 1 };
    
    app->ref_width = (f32)win_width;
    app->ref_height = (f32)win_height;
    
    app->rectb = draw_rectb_create(arena, win, 1024, 32);
    app->cbezier = draw_cbezier_create(arena, win, 1024);
    app->poly = draw_poly_create(arena, win, 256);
    
    app->temp.arena = marena_create(
        &(marena_desc){
            .desired_max_size = MiB(64),
            .desired_block_size = KiB(256)
        }
    );
    app->temp.file_reg = (string8_registry){ 0 };
    
    app->pres = app_pres_parse(arena, app, pres_path);

    return app;
} 


void app_run(marena* arena, app_app* app) {
    str8_reg_init_arr(arena, &app->temp.file_reg);
    
    string8_node* node = app->temp.file_reg.names.first;
    for(u64 i = 0; node != NULL; node = node->next, i++) {
        string8 file = os_file_read(arena, node->str);
        app->temp.file_reg.strings[i] = file;
    }

    for (app_slide_node* slide = app->pres->first_slide; slide != NULL; slide = slide->next) {
        app_objp_file(slide->objs, app->pres->obj_reg, app);
    }

    marena_destroy(app->temp.arena);
    
    draw_rectb_finalize_textures(app->rectb);

    gfx_win_alpha_blend(app->win, true);
    gfx_win_clear_color(app->win, (vec3){ 0 });

    u64 time_prev = os_now_microseconds();
    while (!app->win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        app_pres_update(app->pres, app, delta);

        gfx_win_clear(app->win);

        f32 height = (f32)app->win->height;
        f32 width = height * (app->ref_width / app->ref_height);
        
        if (width > (f32)app->win->width) {
            width = (f32)app->win->width;
            height = width * (app->ref_height / app->ref_width);
        }
        
        f32 top = ((f32)app->win->height - height);
        f32 left = ((f32)app->win->width - width);

        app->win_mat[0 + 0 * 4] = (2.0f * (width / (f32)app->win->width)) / (f32)(app->ref_width);
        app->win_mat[1 + 1 * 4] = (2.0f * (height / (f32)app->win->height)) / -(f32)(app->ref_height);
        app->win_mat[2 + 2 * 4] = -1.0f;
        app->win_mat[3 + 0 * 4] = -1.0f + left / (f32)(app->win->width);
        app->win_mat[3 + 1 * 4] = 1.0f - top / (f32)(app->win->height);
        app->win_mat[3 + 3 * 4] = 1;

        // I could make all of these pointers,
        // but this does not really affect performance
        memcpy(app->cbezier->win_mat, app->win_mat, sizeof(app->win_mat));
        memcpy(app->poly->win_mat, app->win_mat, sizeof(app->win_mat));
        memcpy(app->rectb->win_mat, app->win_mat, sizeof(app->win_mat));

        draw_rectb_push(app->rectb, (rect){ 0, 0, app->ref_width, app->ref_height }, app->bg_col);
        draw_rectb_flush(app->rectb);

        app_pres_draw(app->pres, app);

        draw_rectb_flush(app->rectb);
        draw_cbezier_flush(app->cbezier);

        gfx_win_swap_buffers(app->win);
        gfx_win_process_events(app->win);

        time_prev = time_now;
        u32 sleep_time_ms = MAX(0, 16 - (i64)(delta * 1000));
        os_sleep_milliseconds(sleep_time_ms);
    }
}

void app_destroy(app_app* app) {
    app_pres_destroy(app->pres);

    draw_rectb_destroy(app->rectb);
    draw_cbezier_destroy(app->cbezier);
    draw_poly_destroy(app->poly);

    gfx_win_destroy(app->win);
}
