#ifdef AP_OPENGL

#include "gl_impl.h"

draw_rectb_t* draw_rectb_create(arena_t* arena, gfx_window_t* win, u64 capacity) { 
    draw_rectb_t* batch = CREATE_ZERO_STRUCT(arena, batch, draw_rectb_t);

    batch->data = CREATE_ARRAY(arena, draw_rect_t, capacity);
    batch->capacity = capacity;
    batch->size = 0;

    const char* vertex_source = ""
        "#version 330 core\n"
        "layout (location = 0) in vec2 a_pos_pattern;"
        "layout (location = 1) in vec4 a_quad;"
        "layout (location = 2) in vec3 a_col;"
        "uniform mat2 u_win_mat;"
        "out vec4 col;"
        "void main() {"
        "    col = vec4(a_col, 1);"
        "    vec2 pos = a_quad.xy + (0.5 * a_quad.zw) * (vec2(1) + a_pos_pattern);"
        "    gl_Position = vec4((pos * u_win_mat) + vec2(-1, 1), 0, 1);"
        "}";
    const char* fragment_source = ""
        "#version 330 core\n"
        "in vec4 col;"
        "void main() {\n"
        "    gl_FragColor = col;\n"
        "}";
    batch->gl.shader_program = gl_impl_create_shader_program(vertex_source, fragment_source);

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
        GL_ARRAY_BUFFER, sizeof(draw_rect_t) * (capacity), NULL, GL_DYNAMIC_DRAW
    );
    f32 pos_pattern[] = {
        -1,  1,
         1,  1,
        -1, -1,
         1,  1,
        -1, -1,
         1, -1   
    };
    batch->gl.pos_pattern_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(f32) * 12, &pos_pattern[0], GL_STATIC_DRAW
    );
    
    return batch;
}
void draw_rectb_destroy(draw_rectb_t* batch) {
    glDeleteProgram(batch->gl.shader_program);
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.pos_pattern_buffer);
}

void draw_rectb_push(draw_rectb_t* batch, rect_t rect, vec3_t col) {
    if (batch->size < batch->capacity) {
        batch->data[batch->size++] = (draw_rect_t){
            .rect = rect,
            .col = col
        };
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push(batch, rect, col);
    }
}
void draw_rectb_flush(draw_rectb_t* batch) {
    glUseProgram(batch->gl.shader_program);
    glBindVertexArray(batch->gl.vertex_array);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.pos_pattern_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
        sizeof(draw_rect_t), (void*)offsetof(draw_rect_t, rect));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(draw_rect_t), (void*)offsetof(draw_rect_t, col));
    glVertexAttribDivisor(2, 1);

    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * sizeof(draw_rect_t), batch->data);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)batch->size);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    batch->size = 0;
}

#endif // AP_OPENGL
