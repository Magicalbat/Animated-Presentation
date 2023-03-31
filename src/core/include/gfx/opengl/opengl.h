#include "base/base_defs.h"

#ifdef AP_OPENGL

#ifndef OPENGL_H
#define OPENGL_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "opengl_defs.h"

// TODO: Error handling on both platforms

typedef struct gfx_window gfx_window;
AP_EXPORT void opengl_load_functions(gfx_window* win);

// https://www.khronos.org/opengl/wiki/OpenGL_Error
/*void opengl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    log_level level = LOG_DEBUG;
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            level = LOG_DEBUG;
            break;
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_MEDIUM:
            level = LOG_WARN;
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            level = LOG_ERROR;
            break;
        default: break;
    }
    log_msgf(level, "GL CALLBACK - type = 0x%x, severity = 0x%x, message = %s",
        type, severity, message);
}*/

//glEnable(GL_DEBUG_OUTPUT);
//glDebugMessageCallback(opengl_message_callback, 0);

#ifdef __cplusplus
}
#endif

#endif // OPENGL_H
#endif // AP_OPENGL
