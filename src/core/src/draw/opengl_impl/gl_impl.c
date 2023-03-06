#ifdef AP_OPENGL

#include "draw/opengl_impl/gl_impl.h"

const char* gl_impl_color_frag = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "varying vec4 col;"
    "void main() {"
    "   gl_FragColor = col;"
    "\n}";
#else
    "#version 330 core\n"
    "layout (location = 0) out vec4 out_col;"
    "in vec4 col;"
    "void main() {"
    "    out_col = col;"
    "\n}";
#endif


u32 gl_impl_create_shader_program(const char* vertex_source, const char* fragment_source) {
    u32 vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);
    
    i32 success = GL_TRUE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        log_errorf("Failed to compile vertex shader: %s", info_log);
    }
    
    u32 fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        log_errorf("Failed to compile fragment shader: %s", info_log);
    }

    u32 shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        log_errorf("Failed to link shader: %s", info_log);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

u32 gl_impl_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type) {
    u32 buffer;

    glGenBuffers(1, &buffer);
    glBindBuffer(buffer_type, buffer);
    glBufferData(buffer_type, size, data, draw_type);

    return buffer;
}

void gl_impl_view_mat(gfx_window* win, u32 mat_loc) {
    f32 win_mat[] = {
        2.0f / (f32)win->width, 0,
        0, 2.0f / -((f32)win->height)
    };
    glUniformMatrix2fv(mat_loc, 1, GL_FALSE, &win_mat[0]);
}

#endif // AP_OPENGL
