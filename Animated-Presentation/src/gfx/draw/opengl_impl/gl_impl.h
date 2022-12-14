#ifdef AP_OPENGL

#include "base/base.h"

#include "gfx/gfx.h"
#include "gfx/opengl/opengl.h"
#include "gfx/draw/draw.h"

u32 gl_impl_create_shader_program(const char* vertex_source, const char* fragment_source);
u32 gl_impl_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type);

#endif // AP_OPENGL
