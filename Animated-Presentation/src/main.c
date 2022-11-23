#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    arena_t* perm_arena = arena_create(KB(16));

    gfx_window_t* win = gfx_win_create(perm_arena, 320, 180, str8_lit("Test window"));
    gfx_win_make_current(win);

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    while (!win->info.should_close) {
        gfx_win_process_events(win);

        glClear(GL_COLOR_BUFFER_BIT);

        gfx_win_swap_buffers(win);
    }

    gfx_win_destroy(win);

    arena_free(perm_arena);

    os_main_quit();
    return 0;
}
