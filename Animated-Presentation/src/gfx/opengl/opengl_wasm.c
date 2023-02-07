#ifdef __EMSCRIPTEN__

#include "base/base.h"
#include "gfx/gfx.h"
#include "opengl.h"

void opengl_load_functions(gfx_window* win) { }

gfx_window* gfx_win_create(arena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = CREATE_ZERO_STRUCT(arena, win, gfx_window);

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = 0;
    //attr.depth = 0;
    //attr.stencil = 0;

    win->wasm.ctx = emscripten_webgl_create_context("canvas", &attr);
    //emscripten_set_canvas_element_size("#canvas", width, height);

    win->width = width;
    win->height = height;
    win->title = title;

    return win;
}
void gfx_win_make_current(gfx_window* win) {
    emscripten_webgl_make_context_current(win->wasm.ctx);
}
void gfx_win_destroy(gfx_window* win) {
    emscripten_webgl_destroy_context(win->wasm.ctx);
}

void gfx_win_swap_buffers(gfx_window* win) {
    emscripten_webgl_commit_frame();
}
void gfx_win_process_events(gfx_window* win) {
    // TODO: this
}

void gfx_win_set_size(gfx_window* win, u32 width, u32 height) {
    emscripten_set_canvas_element_size("#canvas", width, height);
}
void gfx_win_set_title(arena* arena, gfx_window* win, string8 title) {
    log_error("GFX set title unsupported in wasm");
}

#endif //__EMSCRIPTEN__
