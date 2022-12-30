#ifndef GFX_H
#define GFX_H

#include "base/base.h"

#if defined(AP_PLATFORM_LINUX)
    #include "X11/Xlib.h"
    #include "X11/Xutil.h"
#elif defined(AP_PLATFORM_WINDOWS)
    #include <Windows.h>
#else
    #error cannot find valid platform
#endif

#define GFX_NUM_KEYS          256
#define GFX_NUM_MOUSE_BUTTONS 5

// TODO: Make this more simlar to draw layer structs

typedef struct gfx_window_info {
    vec2_t mouse_pos;

    b8* new_mouse_buttons;
    b8* old_mouse_buttons;

    b8* new_keys;
    b8* old_keys;

    u32 width;
    u32 height;

    string8_t title;

    b8 should_close;
} gfx_window_info_t;

#ifdef AP_OPENGL
    #include "opengl/opengl.h"
#endif

gfx_window_t* gfx_win_create(arena_t* arena, u32 width, u32 height, string8_t title);
void          gfx_win_make_current(gfx_window_t* win);
void          gfx_win_destroy(gfx_window_t* win);

void gfx_win_swap_buffers(gfx_window_t* win);
void gfx_win_process_events(gfx_window_t* win);

void gfx_win_set_size(gfx_window_t* win, u32 width, u32 height);
void gfx_win_set_title(arena_t* arena, gfx_window_t* win, string8_t title);

#endif // GFX_H
