#ifdef AP_OPENGL

#include "gl_impl.h"

static const char* color_vert_source;
static const char* color_frag_source;

static const char* texture_vert_source;
static const char* texture_frag_source;

static const char* both_vert_source;
static const char* both_frag_source;

static const u32 elem_size[] = {
    sizeof(rect) + sizeof(vec3),
    sizeof(rect) + sizeof(rect),
    sizeof(rect) + sizeof(vec3) + sizeof(rect),
};

draw_rectb* draw_rectb_create_ex(arena* arena, gfx_window* win, u32 capacity, draw_rectb_type type, string8 texture_path) { 
    draw_rectb* batch = CREATE_ZERO_STRUCT(arena, batch, draw_rectb);

    batch->data = arena_alloc(arena, elem_size[type] * capacity);
    batch->capacity = capacity;

    batch->type = type;

    switch (type) {
        case RECTB_COLOR:
            batch->gl.shader_program = gl_impl_create_shader_program(
                color_vert_source, color_frag_source
            );
            break;
        case RECTB_TEXTURE:
            batch->gl.shader_program = gl_impl_create_shader_program(
                texture_vert_source, texture_frag_source
            );
            break;
        case RECTB_BOTH:
            batch->gl.shader_program = gl_impl_create_shader_program(
                both_vert_source, both_frag_source
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

    if (batch->type != RECTB_COLOR) {
        batch->gl.texture = gl_impl_create_texture(arena, texture_path);
    }
 
    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);

    batch->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, elem_size[type] * capacity, NULL, GL_DYNAMIC_DRAW
    );
    f32 pos_pattern[] = {
        0, 1,
        1, 1,
        0, 0,
        1, 0
    };
    
    batch->gl.pos_pattern_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(pos_pattern), &pos_pattern[0], GL_STATIC_DRAW
    );
    
    return batch;
}
void draw_rectb_destroy(draw_rectb* batch) {
    glDeleteProgram(batch->gl.shader_program);
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.pos_pattern_buffer);
    if (batch->type != RECTB_COLOR) {
        glDeleteTextures(1, &batch->gl.texture);
    }
}

void draw_rectb_push_col(draw_rectb* batch, rect draw_rect, vec3 col) {
    if (batch->size < batch->capacity) {
        u32 index = batch->size * elem_size[batch->type];

        *(rect*)(batch->data + index) = draw_rect;
        *(vec3*)(batch->data + index + sizeof(rect)) = col;

        batch->size++;
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push_col(batch, draw_rect, col);
    }
}
void draw_rectb_push_tex(draw_rectb* batch, rect draw_rect, rect tex_rect) {
    if (batch->size < batch->capacity) {
        u32 index = batch->size * elem_size[batch->type];

        *(rect*)(batch->data + index) = draw_rect;
        *(rect*)(batch->data + index + sizeof(rect)) = tex_rect;

        batch->size++;
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push_tex(batch, draw_rect, tex_rect);
    }
}
void draw_rectb_push_both(draw_rectb* batch, rect draw_rect, vec3 col, rect tex_rect) {
    if (batch->size < batch->capacity) {
        u32 index = batch->size * elem_size[batch->type];

        *(rect*)(batch->data + index) = draw_rect;
        *(vec3*)(batch->data + index + sizeof(rect)) = col;
        *(rect*)(batch->data + index + sizeof(rect) + sizeof(vec3)) = tex_rect;

        batch->size++;
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push_both(batch, draw_rect, col, tex_rect);
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
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, elem_size[batch->type], (void*)(0));
    glVertexAttribDivisor(1, 1);

    if (batch->type != RECTB_TEXTURE) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, elem_size[batch->type], (void*)(sizeof(rect)));
        glVertexAttribDivisor(2, 1);
    }
    if (batch->type != RECTB_COLOR) {
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, elem_size[batch->type],
            (void*)(batch->type == RECTB_BOTH ? sizeof(rect) + sizeof(vec3) : sizeof(rect)));
        glVertexAttribDivisor(3, 1);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * elem_size[batch->type], batch->data);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei)batch->size);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

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
    "layout (location = 3) in vec4 a_tex_rect;"
    "uniform mat2 u_win_mat;"
    "out vec2 uv;"
    "void main() {"
    "    uv = a_tex_rect.xy + a_tex_rect.zw * a_pos_pattern;"
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
    "layout (location = 3) in vec4 a_tex_rect;"
    "uniform mat2 u_win_mat;"
    "out vec4 col;"
    "out vec2 uv;"
    "void main() {"
    "    col = vec4(a_col, 1);"
    "    uv = a_tex_rect.xy + a_tex_rect.zw * a_pos_pattern;"
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
