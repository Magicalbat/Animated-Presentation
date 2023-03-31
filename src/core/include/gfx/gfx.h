#ifndef GFX_H
#define GFX_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"

#if defined(__linux__)
    #include "X11/Xlib.h"
    #include "X11/Xutil.h"
#elif defined(_WIN32)
    #if !defined(UNICODE)
        #define UNICODE
    #endif
    #define WIN32_LEAN_AND_MEAN

    #include <Windows.h>
#elif defined(__EMSCRIPTEN__)
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <GLES3/gl3.h>
#else
    #error cannot find valid platform
#endif

#if defined(AP_OPENGL)
    #if defined(__linux__)
        #include <GL/glx.h>
        #include <GL/gl.h>
    #elif defined (_WIN32)
        #include <GL/gl.h>
        
        #define OPENGL_CALLSTYLE __stdcall
    #endif
#endif

#define GFX_NUM_KEYS          256
#define GFX_NUM_MOUSE_BUTTONS 5

typedef struct gfx_window {
    u32 width;
    u32 height;

    vec3 clear_col;

    string8 title;

    b8 should_close;

    #if defined(AP_OPENGL)
        #if defined(__linux__)
            struct {
                Display* display;
                i32 screen;
                GLXFBConfig fb_config;
                Window window;
                GLXContext context;
                Atom del_window;
            } glx;
        #elif defined(_WIN32)
            struct { 
                HINSTANCE h_instance;
                WNDCLASS window_class;
                HWND window;
                HDC device_context;
                HGLRC context;

                u32 keymap[256];
            } wgl;
        #elif defined(__EMSCRIPTEN__)
            struct {
                EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
            } wasm;
        #endif
    #endif

    vec2 mouse_pos;

    b8 mouse_buttons[GFX_NUM_MOUSE_BUTTONS];
    b8 prev_mouse_buttons[GFX_NUM_MOUSE_BUTTONS];

    b8 keys[GFX_NUM_KEYS];
    b8 prev_keys[GFX_NUM_KEYS];

} gfx_window;

AP_EXPORT gfx_window* gfx_win_create(marena* arena, u32 width, u32 height, string8 title);
AP_EXPORT void        gfx_win_make_current(gfx_window* win);
AP_EXPORT void        gfx_win_destroy(gfx_window* win);

AP_EXPORT void gfx_win_clear_color(gfx_window* win, vec3 col);
AP_EXPORT void gfx_win_clear(gfx_window* win);
AP_EXPORT void gfx_win_alpha_blend(gfx_window* win, b32 enable);
AP_EXPORT void gfx_win_swap_buffers(gfx_window* win);
AP_EXPORT void gfx_win_process_events(gfx_window* win);

AP_EXPORT void gfx_win_set_size(gfx_window* win, u32 width, u32 height);
AP_EXPORT void gfx_win_set_title(gfx_window* win, string8 title);

#define GFX_IS_MOUSE_DOWN(win, mb) ( win->mouse_buttons[mb])
#define GFX_IS_MOUSE_UP(win, mb)   (!win->mouse_buttons[mb])
#define GFX_IS_MOUSE_JUST_DOWN(win, mb) (win->mouse_buttons[mb] && !win->prev_mouse_buttons[mb])
#define GFX_IS_MOUSE_JUST_UP(win, mb) (!win->mouse_buttons[mb] && win->prev_mouse_buttons[mb])

#define GFX_IS_KEY_DOWN(win, key) (win->keys[key])
#define GFX_IS_KEY_UP(win, key) (!win->keys[key])
#define GFX_IS_KEY_JUST_DOWN(win, key) (win->keys[key] && !win->prev_keys[key])
#define GFX_IS_KEY_JUST_UP(win, key) (!win->keys[key] && win->prev_keys[key])

typedef enum {
    GFX_MB_LEFT,
    GFX_MB_MIDDLE,
    GFX_MB_RIGHT 
} gfx_mouse_button;

#define GFX_KEY_XLIST \
    X(NONE, = 0) \
    X(BACKSPACE, ) \
    X(TAB, ) \
    X(ENTER, ) \
    X(CAPSLOCK, ) \
    X(ESCAPE, ) \
    X(SPACE, ) \
    X(PAGEUP, ) \
    X(PAGEDOWN, ) \
    X(END, ) \
    X(HOME, ) \
    X(LEFT, ) \
    X(UP, ) \
    X(RIGHT, ) \
    X(DOWN, ) \
    X(INSERT, ) \
    X(DELETE, ) \
    X(0, = '0') \
    X(1, = '1') \
    X(2, = '2') \
    X(3, = '3') \
    X(4, = '4') \
    X(5, = '5') \
    X(6, = '6') \
    X(7, = '7') \
    X(8, = '8') \
    X(9, = '9') \
    X(A, = 'A') \
    X(B, = 'B') \
    X(C, = 'C') \
    X(D, = 'D') \
    X(E, = 'E') \
    X(F, = 'F') \
    X(G, = 'G') \
    X(H, = 'H') \
    X(I, = 'I') \
    X(J, = 'J') \
    X(K, = 'K') \
    X(L, = 'L') \
    X(M, = 'M') \
    X(N, = 'N') \
    X(O, = 'O') \
    X(P, = 'P') \
    X(Q, = 'Q') \
    X(R, = 'R') \
    X(S, = 'S') \
    X(T, = 'T') \
    X(U, = 'U') \
    X(V, = 'V') \
    X(W, = 'W') \
    X(X, = 'X') \
    X(Y, = 'Y') \
    X(Z, = 'Z') \
    X(NUMPAD_0, ) \
    X(NUMPAD_1, ) \
    X(NUMPAD_2, ) \
    X(NUMPAD_3, ) \
    X(NUMPAD_4, ) \
    X(NUMPAD_5, ) \
    X(NUMPAD_6, ) \
    X(NUMPAD_7, ) \
    X(NUMPAD_8, ) \
    X(NUMPAD_9, ) \
    X(NUMPAD_MULTIPLY, ) \
    X(NUMPAD_ADD, ) \
    X(NUMPAD_SUBTRACT, ) \
    X(NUMPAD_DECIMAL, ) \
    X(NUMPAD_DIVIDE, ) \
    X(NUMPAD_ENTER, ) \
    X(F1, ) \
    X(F2, ) \
    X(F3, ) \
    X(F4, ) \
    X(F5, ) \
    X(F6, ) \
    X(F7, ) \
    X(F8, ) \
    X(F9, ) \
    X(F10, ) \
    X(F11, ) \
    X(F12, ) \
    X(NUM_LOCK, ) \
    X(SCROLL_LOCK, ) \
    X(LSHIFT, ) \
    X(RSHIFT, ) \
    X(LCONTROL, ) \
    X(RCONTROL, ) \
    X(LALT, ) \
    X(RALT, ) \
    X(SEMICOLON, ) \
    X(EQUAL, ) \
    X(COMMA, ) \
    X(PERIOD, ) \
    X(MINUS, ) \
    X(FORWARDSLASH, ) \
    X(BACKSLASH, ) \
    X(BACKTICK, ) \
    X(LBRACKET, ) \
    X(RBRACKET, ) \
    X(APOSTROPHE, )

typedef enum {
#define X(key, value) GFX_KEY_##key value,
    GFX_KEY_XLIST
#undef X
} gfx_key;

#ifdef __cplusplus
}
#endif

#endif // GFX_H
