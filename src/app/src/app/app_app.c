#include "app/app.h"

#include "os/os.h"

#include "draw/opengl_impl/gl_impl.h"

#ifdef __EMSCRIPTEN__
EM_JS(void, app_maximize_canvas, (), {
    const canvas = document.querySelector("#APCanvas");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
});
#endif

static void app_make_pres(app_app* app);

app_app* app_create(marena* arena, string8 pres_path, u32 win_width, u32 win_height) {
    app_app* app = CREATE_STRUCT(arena, app_app);

#ifdef __EMSCRIPTEN__
    app_maximize_canvas();
#endif

    gfx_window* win = gfx_win_create(arena, win_width, win_height, STR8_LIT("Animated Presentation"));

    gfx_win_make_current(win);
    opengl_load_functions(win);
    
    app->win = win;
    
    app->bg_col = (vec4d){ .5, .6, .7, 1};
    
    app->ref_width = (f32)win_width;
    app->ref_height = (f32)win_height;
    
    app->pres_arena = marena_create(
        &(marena_desc){ 
            .desired_max_size = MiB(16),
            .desired_block_size = KiB(256)
        }
    );

    app->pres_path = pres_path;
    app->pres_dir_path = pres_path;
    for (i64 i = pres_path.size - 1; i >= 0; i--) {
        if (pres_path.str[i] == '/' || pres_path.str[i] == '\\') {
            break;
        }
        app->pres_dir_path.size--;
    }

    app_make_pres(app);

    return app;
} 

static void app_make_pres(app_app* app) {
    app->rectb = draw_rectb_create(app->pres_arena, app->win_mat, 256, 32, 4);
    app->cbezier = draw_cbezier_create(app->pres_arena, app->win_mat, 256);
    app->poly = draw_poly_create(app->pres_arena, app->win_mat, 128);
    
    app->temp.arena = marena_create(
        &(marena_desc){
            .desired_max_size = MiB(64),
            .desired_block_size = KiB(256)
        }
    );
    app->temp.file_reg = (string8_registry){ 0 };
    
    app->pres = app_pres_parse(app->pres_arena, app, app->pres_path);

#ifndef __EMSCRIPTEN__
    datetime pres_modify_time = os_file_get_stats(app->pres_path).modify_time;
    app->pres_modify_time = datetime_to_sec(pres_modify_time);
#endif
}

static void app_pre_run(app_app* app) {
    str8_reg_init_arr(app->temp.arena, &app->temp.file_reg);
    
    string8_node* node = app->temp.file_reg.names.first;
    for(u64 i = 0; node != NULL; node = node->next, i++) {
        if (node->str.size <= 2)    continue;

        string8 file;

        if (node->str.str[0] == ':' || node->str.str[0] == '/' || node->str.str[0] == '~') {
            file = os_file_read(app->temp.arena, node->str);
        } else {
            marena_temp scratch = marena_scratch_get(NULL, 0);

            u64 total_size = app->pres_dir_path.size + node->str.size;
            string8 total_path = {
                .str = CREATE_ARRAY(scratch.arena, u8, total_size),
                .size = total_size
            };

            memcpy(total_path.str, app->pres_dir_path.str, app->pres_dir_path.size);
            memcpy(total_path.str + app->pres_dir_path.size, node->str.str, node->str.size);

            file = os_file_read(app->temp.arena, total_path);

            marena_scratch_release(scratch);
        }

        app->temp.file_reg.strings[i] = file;
    }

    if (app->pres->global_slide != NULL) {
		app_objp_file(app->pres_arena, app->pres->global_slide->objs, app->pres->obj_reg, app);
    }

    for (app_slide_node* slide = app->pres->first_slide; slide != NULL; slide = slide->next) {
        app_objp_file(app->pres_arena, slide->objs, app->pres->obj_reg, app);
    }

    draw_rectb_finalize_textures(app->rectb);
    
    marena_destroy(app->temp.arena);
}

static void app_reset(app_app* app) {
    u32 slide_index = app->pres->slide_index;
    u32 num_slides = app->pres->num_slides;

    app_pres_destroy(app->pres);

    draw_rectb_destroy(app->rectb);
    draw_cbezier_destroy(app->cbezier);
    draw_poly_destroy(app->poly);

    marena_reset(app->pres_arena);

    app_make_pres(app);
    app_pre_run(app);

    if (app->pres->num_slides >= num_slides) {
        for (u32 i = 0; i < slide_index; i++) {
            app_pres_next_slide(app->pres);
        }
    }
}

void app_run(app_app* app) {
    gfx_win_alpha_blend(app->win, true);
    gfx_win_clear_color(app->win, (vec3){ 0 });

    app_pre_run(app);

    u64 time_prev = os_now_microseconds();
    while (!app->win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        app_pres_update(app->pres, app, delta);

#ifndef __EMSCRIPTEN__
        datetime cur_modify_dt = os_file_get_stats(app->pres_path).modify_time;
        u64 cur_modify_time = datetime_to_sec(cur_modify_dt);

        if (cur_modify_time > app->pres_modify_time) {
            app->pres_modify_time = cur_modify_time;
            app_reset(app);
        }
#endif

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
        
        app->mouse_pos.x = (app->win->mouse_pos.x - left * 0.5) * (f32)(app->ref_width / (app->win->width - left));
        app->mouse_pos.y = (app->win->mouse_pos.y - top * 0.5) * (f32)(app->ref_height / (app->win->height - top));

        gfx_win_clear(app->win);

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

    marena_destroy(app->pres_arena);

    gfx_win_destroy(app->win);
}
