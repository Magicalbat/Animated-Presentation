#ifndef GL_DEFS_H
#define GL_DEFS_H

#include "base/base_defs.h"

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef i8 GLbyte;
typedef u8 GLubyte;
typedef i16 GLshort;
typedef u16 GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef i32 GLclampx;
typedef int GLsizei;
typedef f32 GLfloat;
typedef f32 GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
typedef u16 GLhalf;
typedef u16 GLhalfARB;
typedef i32 GLfixed;
typedef intptr_t GLintptr;
typedef intptr_t GLintptrARB;
typedef i64 GLsizeiptr;
typedef i64 GLsizeiptrARB;
typedef i64 GLint64;
typedef i64 GLint64EXT;
typedef u64 GLuint64;
typedef u64 GLuint64EXT;
typedef struct __GLsync *GLsync;

typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#ifndef OPENGL_CALLSTYLE
    #define OPENGL_CALLSTYLE
#endif
#define X(ret, name, args) typedef ret (OPENGL_CALLSTYLE* gl_func_##name##_t)args;
    #include "opengl_xlist.h"
#undef X

#define X(ret, name, args) extern gl_func_##name##_t name;// = NULL;
    #include "opengl_xlist.h"
#undef X

// https://registry.khronos.org/OpenGL/api/GL/glcorearb.h

#define GL_FALSE                          0
#define GL_TRUE                           1

#define GL_TRIANGLES                      0x0004

#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B

#endif // GL_DEFS_H
