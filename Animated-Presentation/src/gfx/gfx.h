#ifndef GFX_H
#define GFX_H

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

typedef struct {
    u32 width;
    u32 height;

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

gfx_window* gfx_win_create(arena* arena, u32 width, u32 height, string8 title);
void        gfx_win_make_current(gfx_window* win);
void        gfx_win_destroy(gfx_window* win);

void gfx_win_swap_buffers(gfx_window* win);
void gfx_win_process_events(gfx_window* win);

void gfx_win_set_size(gfx_window* win, u32 width, u32 height);
void gfx_win_set_title(arena* arena, gfx_window* win, string8 title);

#define GFX_MOUSE_DOWN(win, mb) ( win->mouse_buttons[mb])
#define GFX_MOUSE_UP(win, mb)   (!win->mouse_buttons[mb])
#define GFX_MOUSE_JUST_DOWN(win, mb) (win->mouse_buttons[mb] && !win->prev_mouse_buttons[mb])
#define GFX_MOUSE_JUST_UP(win, mb) (!win->mouse_buttons[mb] && win->prev_mouse_buttons[mb])

#define GFX_MB_LEFT   0
#define GFX_MB_MIDDLE 1
#define GFX_MB_RIGHT  2

#endif // GFX_H
