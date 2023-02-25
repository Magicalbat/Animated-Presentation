#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"
#include "draw/draw.h"
#include "draw/opengl_impl/gl_impl.h"

// TODO: enable -Wall and fix warnings

#define WIN_SCALE 1.5
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init((log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });
    
    marena* perm_arena = marena_create(&(marena_desc){
        .desired_max_size = MiB(4),
        .desired_block_size = KiB(64)
    });

    gfx_window* win = gfx_win_create(
        perm_arena,
        WIDTH, HEIGHT,
        STR8_LIT("Test window")
    );
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    //log_infof("GL Vender: %s",   glGetString(GL_VENDOR));
    //log_infof("GL Renderer: %s", glGetString(GL_RENDERER));
    //log_infof("GL Version: %s",  glGetString(GL_VERSION));

    gfx_win_alpha_blend(win, true);
    gfx_win_clear_color(win, (vec3){ 0.5f, 0.6f, 0.7f });

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    while (!win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_clear(win);

        gfx_win_swap_buffers(win);
        gfx_win_process_events(win);

        time_prev = time_now;
        os_sleep_milliseconds(16);
    }

    gfx_win_destroy(win);
    
    marena_destroy(perm_arena);
    log_quit();
    os_main_quit();

    return 0;
}
