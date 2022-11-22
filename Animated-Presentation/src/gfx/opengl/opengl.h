#ifdef AP_OPENGL

#ifndef OPENGL_H
#define OPENGL_H

#if defined(AP_PLATFORM_LINUX)
    #include <GL/glx.h>
    #include <GL/gl.h>
#elif defined (AP_PLATFORM_WINDOWS)
    #include <GL/gl.h>
#endif

//typedef struct gfx_window_info gfx_window_info_t;

typedef struct {
    gfx_window_info_t info;

    #if defined(AP_PLATFORM_LINUX)
        struct {
            Display* display;
            i32 screen;
            GLXFBConfig fb_config;
            Window window;
            GLXContext context;
        } glx;
    #elif defined(AP_PLATFORM_WINDOWS)
        struct { } wgl;
    #endif
} gfx_window_t;

#endif // OPENGL_H
#endif // AP_OPENGL
