#ifndef GL_IMPL_H
#define GL_IMPL_H

#include "base/base.h"
#include "os/os.h"
#include "parse/parse.h"

#include "gfx/gfx.h"
#include "gfx/opengl/opengl.h"
#include "draw/draw.h"

typedef enum {
    IMPL_GL_LINEAR,
    IMPL_GL_NEAREST
} impl_gl_filter;

// TODO: add basic color frag shader here

u32 gl_impl_create_shader_program(const char* vertex_source, const char* fragment_source);
u32 gl_impl_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type);
u32 gl_impl_create_texture_ex(arena* arena, string8 file_path, impl_gl_filter filter);
#define gl_impl_create_texture(arena, file_path) gl_impl_create_texture_ex(arena, file_path, IMPL_GL_LINEAR)

#endif // GL_IMPL_H
