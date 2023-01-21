#ifdef AP_OPENGL

#include "gl_impl.h"

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
u32 gl_impl_create_texture_ex(arena* arena, string8 file_path, impl_gl_filter filter) {
    arena_temp temp = arena_temp_begin(arena);
    string8 file = os_file_read(temp.arena, file_path);
    if (file.size == 0) { 
        log_errorf("Failed to load texture at \"%.*s\"", (int)file_path.size, file_path.str);
        return -1;
    }
    image img = { 0 };
    
    img = parse_png(arena, file);
    if (!img.valid) {
        log_errorf("Failed to load texture at \"%.*s\"", (int)file_path.size, file_path.str);
        return -1;
    }
    
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    if (filter == IMPL_GL_LINEAR) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else if (filter == IMPL_GL_NEAREST) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    
    u32 color_type = img.channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, color_type, img.width, img.height, 0, color_type, GL_UNSIGNED_BYTE, img.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    arena_temp_end(temp);

    return texture;
}

#endif // AP_OPENGL
