#ifdef AP_OPENGL

#ifndef OPENGL_H
#define OPENGL_H

#include "gfx/gfx.h"

#if defined(AP_PLATFORM_LINUX)
    #include <GL/glx.h>
    #include <GL/gl.h>
#elif defined (AP_PLATFORM_WINDOWS)
    #include <GL/gl.h>
    
    #define OPENGL_CALLSTYLE __stdcall
#endif

#include "opengl_defs.h"

// TODO: Error handling on both platforms

void opengl_load_functions(gfx_window_t* win);

#endif // OPENGL_H
#endif // AP_OPENGL
