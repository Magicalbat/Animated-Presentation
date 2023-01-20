#ifdef AP_OPENGL

#include "gl_impl.h"

static const char* color_vert_source;
static const char* color_frag_source;

static const char* texture_vert_source;
static const char* texture_frag_source;

static const char* both_vert_source;
static const char* both_frag_source;

draw_rectb* draw_rectb_create_ex(arena* arena, gfx_window* win, u64 capacity, draw_rectb_type type, string8 texture_path) { 
    draw_rectb* batch = CREATE_ZERO_STRUCT(arena, batch, draw_rectb);

    batch->data = CREATE_ARRAY(arena, draw_rect, capacity);
    batch->capacity = capacity;
    batch->size = 0;

    switch (type) {
        case RECTB_COLOR:
            batch->gl.shader_program = gl_impl_create_shader_program(
                color_vert_source,
                color_frag_source
            );
            break;
        case RECTB_TEXTURE:
            batch->gl.shader_program = gl_impl_create_shader_program(
                texture_vert_source,
                texture_frag_source
            );
            break;
        case RECTB_BOTH:
            batch->gl.shader_program = gl_impl_create_shader_program(
                both_vert_source,
                both_frag_source
            );
            break;
    }

    glUseProgram(batch->gl.shader_program);
    
    u32 win_mat_loc = glGetUniformLocation(batch->gl.shader_program, "u_win_mat");
    f32 win_mat[] = {
        2.0f / (f32)win->width, 0,
        0, 2.0f / -((f32)win->height)
    };
    glUniformMatrix2fv(win_mat_loc, 1, GL_FALSE, &win_mat[0]);
 
    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);

    batch->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(draw_rect) * (capacity), NULL, GL_DYNAMIC_DRAW
    );
    f32 pos_pattern[] = {
        0, 1,
        1, 1,
        0, 0,
        1, 1,
        0, 0,
        1, 0
    };
    
    batch->gl.pos_pattern_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(f32) * 12, &pos_pattern[0], GL_STATIC_DRAW
    );
    
    return batch;
}
void draw_rectb_destroy(draw_rectb* batch) {
    glDeleteProgram(batch->gl.shader_program);
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.pos_pattern_buffer);
}

void draw_rectb_push(draw_rectb* batch, rect rect, vec3 col) {
    if (batch->size < batch->capacity) {
        batch->data[batch->size++] = (draw_rect){
            .rect = rect,
            .col = col
        };
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push(batch, rect, col);
    }
}
void draw_rectb_flush(draw_rectb* batch) {
    glUseProgram(batch->gl.shader_program);
    glBindVertexArray(batch->gl.vertex_array);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.pos_pattern_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
        sizeof(draw_rect), (void*)offsetof(draw_rect, rect));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(draw_rect), (void*)offsetof(draw_rect, col));
    glVertexAttribDivisor(2, 1);

    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * sizeof(draw_rect), batch->data);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)batch->size);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    batch->size = 0;
}

static const char* color_vert_source = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos_pattern;"
    "layout (location = 1) in vec4 a_quad;"
    "layout (location = 2) in vec3 a_col;"
    "uniform mat2 u_win_mat;"
    "out vec4 col;"
    "void main() {"
    "    col = vec4(a_col, 1);"
    "    vec2 pos = a_quad.xy + a_quad.zw * a_pos_pattern;"
    "    gl_Position = vec4((pos * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
        
static const char* color_frag_source = ""
    "#version 330 core\n"
    "in vec4 col;"
    "void main() {"
    "    gl_FragColor = col;"
    "\n}";

static const char* texture_vert_source = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos_pattern;"
    "layout (location = 1) in vec4 a_quad;"
    "layout (location = 3) in vec2 a_uv;"
    "uniform mat2 u_win_mat;"
    "out vec2 uv;"
    "void main() {"
    "    uv = a_uv;"
    "    vec2 pos = a_quad.xy + a_quad.zw * a_pos_pattern;"
    "    gl_Position = vec4((pos * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
        
static const char* texture_frag_source = ""
    "#version 330 core\n"
    "in vec2 uv;"
    "uniform sampler2D texture1;"
    "void main() {"
    "    gl_FragColor = texture(texture1, uv);"
    "\n}";

static const char* both_vert_source = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos_pattern;"
    "layout (location = 1) in vec4 a_quad;"
    "layout (location = 2) in vec3 a_col;"
    "layout (location = 3) in vec2 a_uv;"
    "uniform mat2 u_win_mat;"
    "out vec4 col;"
    "out vec2 uv;"
    "void main() {"
    "    col = vec4(a_col, 1);"
    "    uv = a_uv;"
    "    vec2 pos = a_quad.xy + a_quad.zw * a_pos_pattern;"
    "    gl_Position = vec4((pos * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
        
static const char* both_frag_source = ""
    "#version 330 core\n"
    "in vec4 col;"
    "in vec2 uv;"
    "uniform sampler2D texture1;"
    "void main() {"
    "    gl_FragColor = texture(texture1, uv) * col;"
    "\n}";


#endif // AP_OPENGL