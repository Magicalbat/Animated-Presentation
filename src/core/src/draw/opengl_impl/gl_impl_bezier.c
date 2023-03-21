#ifdef AP_OPENGL

#include "draw/opengl_impl/gl_impl.h"

static const char* vert_source;

draw_cbezier* draw_cbezier_create(marena* arena, gfx_window* win, u32 capacity) {
    draw_cbezier* draw_cb = CREATE_ZERO_STRUCT(arena, draw_cbezier);

    draw_cb->capacity = capacity;
    draw_cb->indices = CREATE_ARRAY(arena, u32, capacity * 6);
    draw_cb->vertices = CREATE_ARRAY(arena, cb_vertex, capacity * 4);

    draw_cb->gl.shader_program = gl_impl_create_shader_program(vert_source, gl_impl_color_frag);

    glUseProgram(draw_cb->gl.shader_program);
    
    draw_cb->gl.win_mat_loc = glGetUniformLocation(draw_cb->gl.shader_program, "u_win_mat");
    
#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &draw_cb->gl.vertex_array);
    glBindVertexArray(draw_cb->gl.vertex_array);
#endif

    draw_cb->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(cb_vertex) * (capacity * 3), NULL, GL_DYNAMIC_DRAW
    );
    
    draw_cb->gl.index_buffer = gl_impl_create_buffer(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * (capacity * 6), NULL, GL_DYNAMIC_DRAW
    );

    return draw_cb;
}
void draw_cbezier_destroy(draw_cbezier* draw_cb) {
    glDeleteProgram(draw_cb->gl.shader_program);
#ifndef __EMSCRIPTEN__
    glDeleteVertexArrays(1, &draw_cb->gl.vertex_array);
#endif
    glDeleteBuffers(1, &draw_cb->gl.vertex_buffer);
    glDeleteBuffers(1, &draw_cb->gl.index_buffer);
}

static void draw_cbezier_push_internal(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec4 start_col, vec4 end_col) {
    f32 estimate_len = 
        vec2_len(vec2_sub(bezier->p1, bezier->p0)) +
        vec2_len(vec2_sub(bezier->p2, bezier->p1)) +
        vec2_len(vec2_sub(bezier->p3, bezier->p2));

    u32 num_segs = MAX(0, (u32)(estimate_len * 0.1));
    num_segs = MIN(num_segs, draw_cb->capacity);

    if ((draw_cb->index_pos / 6) + 1 + num_segs > draw_cb->capacity) {
        draw_cbezier_flush(draw_cb);
    }

    vec3 scol = rgb_to_hsv((vec3){ start_col.x, start_col.y, start_col.z });
    vec3 ecol = rgb_to_hsv((vec3){ end_col.x, end_col.y, end_col.z });

    f32 half_width = width * 0.5f;

    vec2 pos = cbezier_calc(bezier, 0);
    vec2 perp = vec2_nrm(vec2_prp(cbezier_calcd(bezier, 0)));
    vec3 col3 = scol;
    vec4 col = { 0 };

    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_add(pos, vec2_mul(perp, half_width)),
        .col = start_col
    };
    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_add(pos, vec2_mul(perp, -half_width)),
        .col = start_col
    };

    f32 step = 1.0f / (f32)(num_segs);
    f32 t = step;
    for (u32 i = 1; i < num_segs && t < 1.0f; i++, t += step) {
        pos = cbezier_calc(bezier, t);
        perp = vec2_mul(vec2_nrm(vec2_prp(cbezier_calcd(bezier, t))), half_width);
        col3 = hsv_to_rgb(vec3_add(vec3_mul(scol, 1 - t), vec3_mul(ecol, t)));
        col.x = col3.x;
        col.y = col3.y;
        col.z = col3.z;
        col.w = LERP(start_col.w, end_col.w, t);

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
        .col = end_col
    };
    draw_cb->vertices[draw_cb->vertex_pos++] = (cb_vertex){
        .pos = vec2_sub(pos, perp),
        .col = end_col
    };

    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 4;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
    
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 3;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 2;
    draw_cb->indices[draw_cb->index_pos++] = draw_cb->vertex_pos - 1;
}
void draw_cbezier_push_grad(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec4d start_col, vec4d end_col) {
    draw_cbezier_push_internal(
        draw_cb, bezier, width,
        (vec4){ start_col.x, start_col.y, start_col.z, start_col.w },
        (vec4){ end_col.x, end_col.y, end_col.z, end_col.w }
    );
}
void draw_cbezier_push(draw_cbezier* draw_cb, cbezier* bezier, u32 width, vec4d col) {
    draw_cbezier_push_internal(
        draw_cb, bezier, width,
        (vec4){ col.x, col.y, col.z, col.w },
        (vec4){ col.x, col.y, col.z, col.w }
    );
}

void draw_cbezier_flush(draw_cbezier* draw_cb) {
    glUseProgram(draw_cb->gl.shader_program);
    //gl_impl_view_mat(draw_cb->win, draw_cb->gl.win_mat_loc);
    glUniformMatrix4fv(draw_cb->gl.win_mat_loc, 1, GL_FALSE, draw_cb->win_mat);

#ifndef __EMSCRIPTEN__
    glBindVertexArray(draw_cb->gl.vertex_array);
#endif

    glBindBuffer(GL_ARRAY_BUFFER, draw_cb->gl.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, draw_cb->vertex_pos * sizeof(cb_vertex), draw_cb->vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_cb->gl.index_buffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, draw_cb->index_pos * sizeof(u32), draw_cb->indices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(cb_vertex), (void*)(offsetof(cb_vertex, pos)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(cb_vertex), (void*)(offsetof(cb_vertex, col)));

    glDrawElements(GL_TRIANGLES, draw_cb->index_pos, GL_UNSIGNED_INT, NULL);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    draw_cb->vertex_pos = 0;
    draw_cb->index_pos = 0;
}

static const char* vert_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "attribute vec2 a_pos;"
    "attribute vec4 a_col;"
    "uniform mat4 u_win_mat;"
    "varying vec4 col;"
    "void main() {"
    "    col = a_col;"
    "    gl_Position = vec4(a_pos, 0, 1) * u_win_mat;"
    "\n}";
#else
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "layout (location = 1) in vec4 a_col;"
    "uniform mat4 u_win_mat;"
    "out vec4 col;"
    "void main() {"
    "    col = a_col;"
    "    gl_Position = vec4(a_pos, 0, 1) * u_win_mat;"
    "\n}";
#endif

#endif // AP_OPENGL
