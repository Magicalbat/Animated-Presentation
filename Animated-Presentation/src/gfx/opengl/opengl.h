#ifdef AP_OPENGL

#ifndef OPENGL_H
#define OPENGL_H

#if defined(AP_PLATFORM_LINUX)
    #include <GL/glx.h>
    #include <GL/gl.h>
#elif defined (AP_PLATFORM_WINDOWS)
    #include <GL/gl.h>
    
    #define OPENGL_CALLSTYLE __stdcall
#endif

#include "opengl_defs.h"

// TODO: Error handling on both platforms

typedef struct {
    gfx_window_info_t info;

    #if defined(AP_PLATFORM_LINUX)
        struct {
            Display* display;
            i32 screen;
            GLXFBConfig fb_config;
            Window window;
            GLXContext context;
            Atom del_window;
        } glx;
    #elif defined(AP_PLATFORM_WINDOWS)
        struct { 
            HINSTANCE h_instance;
            WNDCLASS window_class;
            HWND window;
            HDC device_context;
            HGLRC context;
        } wgl;
    #endif
} gfx_window_t;

void opengl_load_functions(gfx_window_t* win);

#endif // OPENGL_H
#endif // AP_OPENGL
