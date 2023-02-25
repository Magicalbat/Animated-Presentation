#ifdef __EMSCRIPTEN__

#include "base/base.h"
#include "gfx/gfx.h"
#include "opengl.h"

EM_BOOL on_mouse_move(int event_type, const EmscriptenMouseEvent* e, void* win_ptr) {
    gfx_window* win = (gfx_window*)win_ptr;

    win->mouse_pos.x = (f32)e->targetX;
    win->mouse_pos.y = (f32)e->targetY;

    return true;
}

EM_BOOL on_mouse_down(int event_type, const EmscriptenMouseEvent* e, void* win_ptr) {
    gfx_window* win = (gfx_window*)win_ptr;

    win->mouse_buttons[e->button] = true;

    return true;
}

EM_BOOL on_mouse_up(int event_type, const EmscriptenMouseEvent* e, void* win_ptr) {
    gfx_window* win = (gfx_window*)win_ptr;

    win->mouse_buttons[e->button] = false;

    return true;
}

void opengl_load_functions(gfx_window* win) { }

gfx_window* gfx_win_create(marena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = CREATE_ZERO_STRUCT(arena, gfx_window);

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = 0;
    attr.depth = 0;
    attr.stencil = 0;
    attr.explicitSwapControl = true;
	attr.renderViaOffscreenBackBuffer = true;

    win->wasm.ctx = emscripten_webgl_create_context("canvas", &attr);
    emscripten_set_canvas_element_size("#canvas", width, height);

    emscripten_set_mousemove_callback("#canvas", win, false, on_mouse_move);
    emscripten_set_mousedown_callback("#canvas", win, false, on_mouse_down);
    emscripten_set_mouseup_callback("#canvas", win, false, on_mouse_up);

    win->width = width;
    win->height = height;
    win->title = title;

    return win;
}
void gfx_win_make_current(gfx_window* win) {
    emscripten_webgl_make_context_current(win->wasm.ctx);
    
    glViewport(0, 0, win->width, win->height);
}
void gfx_win_destroy(gfx_window* win) {
    emscripten_webgl_destroy_context(win->wasm.ctx);
}

void gfx_win_clear_color(gfx_window* win, vec3 col) {
    win->clear_col = col;
    glClearColor(col.x, col.y, col.z, 1.0f);
}
void gfx_win_clear(gfx_window* win) {
    glClear(GL_COLOR_BUFFER_BIT);
}
void gfx_win_alpha_blend(gfx_window* win, b32 enable) {
    if (enable) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    } else {
        glDisable(GL_BLEND);
    }
}
void gfx_win_swap_buffers(gfx_window* win) {
    emscripten_webgl_commit_frame();
}
void gfx_win_process_events(gfx_window* win) {
    memcpy(win->prev_mouse_buttons, win->mouse_buttons, GFX_NUM_MOUSE_BUTTONS);
    memcpy(win->prev_keys, win->keys, GFX_NUM_KEYS);
}

void gfx_win_set_size(gfx_window* win, u32 width, u32 height) {
    emscripten_set_canvas_element_size("#canvas", width, height);
}
void gfx_win_set_title( gfx_window* win, string8 title) {
    log_error("GFX set title unsupported in wasm");
}

#endif //__EMSCRIPTEN__
