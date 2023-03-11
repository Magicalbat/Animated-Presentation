#ifdef AP_OPENGL

#include "draw/opengl_impl/gl_impl.h"

static const char* vert_source;
// gl_impl_color_frag

draw_polygon* draw_poly_create(marena* arena, gfx_window* win, u32 max_verts) {
    draw_polygon* poly = CREATE_ZERO_STRUCT(arena, draw_polygon);

    poly->win = win;

    poly->max_verts = max_verts;
    poly->verts = CREATE_ARRAY(arena, vec2, max_verts);
    poly->indices = CREATE_ARRAY(arena, u32, max_verts * 3);

    poly->gl.shader_program = gl_impl_create_shader_program(vert_source, gl_impl_color_frag);

    glUseProgram(poly->gl.shader_program);

    poly->gl.win_mat_loc = glGetUniformLocation(poly->gl.shader_program, "u_win_mat");
        
    poly->gl.col_loc = glGetUniformLocation(poly->gl.shader_program, "u_col");
    glUniform3f(poly->gl.col_loc, 1.0f, 1.0f, 1.0f);
    
    poly->gl.offset_loc = glGetUniformLocation(poly->gl.shader_program, "u_offset");
    glUniform2f(poly->gl.offset_loc, 0.0f, 0.0f);
    
#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &poly->gl.vertex_array);
    glBindVertexArray(poly->gl.vertex_array);
#endif
    
    poly->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(vec2) * max_verts, NULL, GL_DYNAMIC_DRAW
    );
    
    poly->gl.index_buffer = gl_impl_create_buffer(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * max_verts * 3, NULL, GL_DYNAMIC_DRAW
    );

    return poly;
}
void draw_poly_destroy(draw_polygon* poly) {
    glDeleteProgram(poly->gl.shader_program);
#ifndef __EMSCRIPTEN__
    glDeleteVertexArrays(1, &poly->gl.vertex_array);
#endif
    glDeleteBuffers(1, &poly->gl.vertex_buffer);
    glDeleteBuffers(1, &poly->gl.index_buffer);
}

static void poly_gl_setup(draw_polygon* poly, vec3 col, vec2 offset) {
    glUseProgram(poly->gl.shader_program);

    gl_impl_view_mat(poly->win, poly->gl.win_mat_loc);
    glUniform3f(poly->gl.col_loc, col.x, col.y, col.z);
    glUniform2f(poly->gl.offset_loc, offset.x, offset.y);

#ifndef __EMSCRIPTEN__
    glBindVertexArray(poly->gl.vertex_array);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, poly->gl.vertex_buffer);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

}
static void poly_gl_end(draw_polygon* poly) {
    glDisableVertexAttribArray(0);
}

void draw_poly_conv_list(draw_polygon* poly, vec3 col, vec2 offset, vec2_list list) {
    if (list.size > poly->max_verts) {
        log_errorf("Cannot draw polygon of %u (max is %u)", list.size, poly->max_verts);
        return;
    }

    u32 i = 0;
    for (vec2_node* node = list.first; node != NULL; node = node->next) {
        poly->verts[i++] = node->v;
    }

    poly_gl_setup(poly, col, offset);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * list.size, poly->verts);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, list.size);

    poly_gl_end(poly);
}
void draw_poly_conv_arr(draw_polygon* poly, vec3 col, vec2 offset, vec2_arr arr) {
    if (arr.size > poly->max_verts) {
        log_errorf("Cannot draw polygon of %u (max is %u)", arr.size, poly->max_verts);
        return;
    }

    poly_gl_setup(poly, col, offset);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * arr.size, arr.data);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, arr.size);

    poly_gl_end(poly);
}

//void draw_poly_list(draw_polygon* poly, vec3 col, vec2_list list) {
//    log_error("TODO: draw_poly_list");
//}
//void draw_poly_arr(draw_polygon* poly, vec3 col, vec2_arr arr) {
//    log_error("TODO: draw_poly_arr");
//}

static const char* vert_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "attribute vec2 a_pos;"
    "uniform mat2 u_win_mat;"
    "uniform vec3 u_col;"
    "uniform vec2 u_offset;"
    "varying vec4 col;"
    "void main() {"
    "    col = vec4(u_col, 1);"
    "    gl_Position = vec4(((a_pos + u_offset) * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
#else
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "uniform mat2 u_win_mat;"
    "uniform vec3 u_col;"
    "uniform vec2 u_offset;"
    "out vec4 col;"
    "void main() {"
    "    col = vec4(u_col, 1);"
    "    gl_Position = vec4(((a_pos + u_offset) * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
#endif

#endif // AP_OPENGL