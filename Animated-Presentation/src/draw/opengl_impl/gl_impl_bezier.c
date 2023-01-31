#ifdef AP_OPENGL

#include "gl_impl.h"

static const char* vert_source;
// gl_impl_color_frag

draw_cbezier* draw_cbezier_create(arena* arena, gfx_window* win, u32 capacity) {
    draw_cbezier* draw_cb = CREATE_ZERO_STRUCT(arena, draw_cb, draw_cbezier);

    draw_cb->capacity = capacity;
    draw_cb->indices = CREATE_ARRAY(arena, u32, capacity * 6);
    draw_cb->vertices = CREATE_ARRAY(arena, cb_vertex, capacity * 4);

    draw_cb->gl.shader_program = gl_impl_create_shader_program(vert_source, gl_impl_color_frag);

    glUseProgram(draw_cb->gl.shader_program);
    
    u32 win_mat_loc = glGetUniformLocation(draw_cb->gl.shader_program, "u_win_mat");
    f32 win_mat[] = {
        2.0f / (f32)win->width, 0,
        0, 2.0f / -((f32)win->height)
    };
    glUniformMatrix2fv(win_mat_loc, 1, GL_FALSE, &win_mat[0]);

    glGenVertexArrays(1, &draw_cb->gl.vertex_array);
    glBindVertexArray(draw_cb->gl.vertex_array);

    draw_cb->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(cb_vertex) * capacity * 4, NULL, GL_DYNAMIC_DRAW
    );
    
    draw_cb->gl.index_buffer = gl_impl_create_buffer(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * capacity * 6, NULL, GL_DYNAMIC_DRAW
    );

    return draw_cb;
}
void draw_cbezier_destroy(draw_cbezier* draw_cb) {
    glDeleteProgram(draw_cb->gl.shader_program);
    glDeleteVertexArrays(1, &draw_cb->gl.vertex_array);
    glDeleteBuffers(1, &draw_cb->gl.vertex_buffer);
    glDeleteBuffers(1, &draw_cb->gl.index_buffer);
}

void draw_cbezier_push(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec3 col) {
    f32 estimate_len = 
        vec2_len(vec2_sub(bezier->p1, bezier->p0)) +
        vec2_len(vec2_sub(bezier->p2, bezier->p1)) +
        vec2_len(vec2_sub(bezier->p3, bezier->p2));

    u32 num_segs = MAX(0, (u32)(estimate_len * 0.1));

    if (num_segs > draw_cb->capacity * 4) {
        log_error("Bezier is too bit to draw");
        return;
    }
    if ((draw_cb->vertex_pos / 4) + 1 + num_segs > draw_cb->capacity) {
        draw_cbezier_flush(draw_cb);
    }

    f32 half_width = width * 0.5f;

    vec2 pos = cbezier_calc(bezier, 0);
    vec2 perp = vec2_nrm(vec2_prp(cbezier_calcd(bezier, 0)));

    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_add(pos, vec2_mul(perp, half_width)),
        .col = col
    };
    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_add(pos, vec2_mul(perp, -half_width)),
        .col = col
    };

    f32 step = 1.0f / (f32)(num_segs);
    f32 t = step;
    for (u32 i = 1; i < num_segs && t < 1.0f; i++, t += step) {
        vec2 pos = cbezier_calc(bezier, t);
        vec2 perp = vec2_mul(vec2_nrm(vec2_prp(cbezier_calcd(bezier, t))), half_width);

        draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
            .pos = vec2_add(pos, perp),
            .col = col
        };
        draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
            .pos = vec2_sub(pos, perp),
            .col = col
        };

        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 4;
        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
        
        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
        draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 1;
    }

    pos = cbezier_calc(bezier, 1);
    perp = vec2_mul(vec2_nrm(vec2_prp(cbezier_calcd(bezier, 1))), half_width);

    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_add(pos, perp),
        .col = col
    };
    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_sub(pos, perp),
        .col = col
    };

    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 4;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
    
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 1;
}
void draw_cbezier_push_grad(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec3 start_col, vec3 end_col) {
    log_error("TODO: cbezier push grad");
}
void draw_cbezier_flush(draw_cbezier* draw_cb) {
    glUseProgram(draw_cb->gl.shader_program);
    glBindVertexArray(draw_cb->gl.vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, draw_cb->gl.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, draw_cb->vertex_pos * sizeof(cb_vertex), draw_cb->vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_cb->gl.index_buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, draw_cb->index_pos * sizeof(u32), draw_cb->indices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(cb_vertex), (void*)(offsetof(cb_vertex, pos)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cb_vertex), (void*)(offsetof(cb_vertex, col)));

    glDrawElements(GL_TRIANGLES, draw_cb->index_pos, GL_UNSIGNED_INT, NULL);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    draw_cb->vertex_pos = 0;
    draw_cb->index_pos = 0;
}

static const char* vert_source = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "layout (location = 1) in vec3 a_col;"
    "uniform mat2 u_win_mat;"
    "out vec4 col;"
    "void main() {"
    "    col = vec4(a_col, 1);"
    "    gl_Position = vec4((a_pos * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";

#endif // AP_OPENGL
