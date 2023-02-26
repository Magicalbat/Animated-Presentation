#ifndef GL_IMPL_H
#define GL_IMPL_H

#include "base/base.h"
#include "os/os.h"
#include "parse/parse.h"

#include "gfx/gfx.h"
#include "gfx/opengl/opengl.h"
#include "draw/draw.h"

extern const char* gl_impl_color_frag;

u32 gl_impl_create_shader_program(const char* vertex_source, const char* fragment_source);
u32 gl_impl_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type);
void gl_impl_view_mat(gfx_window* win, u32 mat_loc);

#endif // GL_IMPL_H
