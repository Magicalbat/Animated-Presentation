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

// TODO: Make this more simlar to draw layer structs

typedef struct {
    vec2_t mouse_pos;

    b8* new_mouse_buttons;
    b8* old_mouse_buttons;

    b8* new_keys;
    b8* old_keys;

    u32 width;
    u32 height;

    string8_t title;

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
        #endif
    #endif
} gfx_window_t;

gfx_window_t* gfx_win_create(arena_t* arena, u32 width, u32 height, string8_t title);
void          gfx_win_make_current(gfx_window_t* win);
void          gfx_win_destroy(gfx_window_t* win);

void gfx_win_swap_buffers(gfx_window_t* win);
void gfx_win_process_events(gfx_window_t* win);

void gfx_win_set_size(gfx_window_t* win, u32 width, u32 height);
void gfx_win_set_title(arena_t* arena, gfx_window_t* win, string8_t title);

#endif // GFX_H
