#ifndef GL_IMPL_H
#define GL_IMPL_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "os/os.h"
#include "parse/parse.h"

#include "gfx/gfx.h"
#include "gfx/opengl/opengl.h"
#include "draw/draw.h"

extern const char* gl_impl_color_frag;

AP_EXPORT u32 gl_impl_create_shader_program(const char* vertex_source, const char* fragment_source);
AP_EXPORT u32 gl_impl_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type);

#ifdef __cplusplus
}
#endif

#endif // GL_IMPL_H
