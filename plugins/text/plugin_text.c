#include "ap_core.h"
#include "app/app.h"

typedef struct {
    string8 source;
} pres_font;

#if 0

#include "draw/opengl_impl/gl_impl.h"

static u32 shader_prog;
static u32 vertex_array;
static u32 vertex_buffer;
static u32 texture;
static const char* vert_source = ""
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos;"
    "layout (location = 1) in vec2 a_uv;"
    "layout (location = 2) in vec4 a_col;"
    "out vec4 col;"
    "out vec2 uv;"
    "void main() {"
    "   col = a_col;"
    "   uv = a_uv;"
    "   gl_Position = vec4(a_pos, 0, 1);"
    "\n}";
static const char* frag_source = ""
    "#version 330 core\n"
    "layout (location = 0) out vec4 out_col;"
    "in vec4 col;"
    "in vec2 uv;"
    "uniform sampler2D u_texture;"
    "void main() {"
    "   out_col = texture(u_texture, uv) * col;"
    "\n}";

void test_desc_init(marena* arena, app_app* app, void* custon_data) {
    shader_prog = gl_impl_create_shader_program(vert_source, frag_source);

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    f32 data[] = {
        -0.5, -0.5,   0.0, 1.0,  1.0, 0.0, 0.0, 1.0,
        -0.5,  0.5,   0.0, 0.0,  0.0, 1.0, 0.0, 1.0,
         0.5,  0.5,   1.0, 0.0,  0.0, 0.0, 1.0, 1.0,

        -0.5, -0.5,   0.0, 1.0,  1.0, 0.0, 0.0, 1.0,
         0.5,  0.5,   1.0, 0.0,  0.0, 0.0, 1.0, 1.0,
         0.5, -0.5,   1.0, 1.0,  0.0, 1.0, 1.0, 0.5,
    };
    vertex_buffer = gl_impl_create_buffer(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    u32 pixel = 0xffffffff;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);

    glGenerateMipmap(GL_TEXTURE_2D);

    u32 texture_loc = glGetUniformLocation(shader_prog, "u_texture");
    glUniform1i(texture_loc, texture);
}

void test_desc_destroy(void* custon_data) {
    glDeleteProgram(shader_prog);
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteTextures(1, &texture);
}

void test_draw(app_app* app, void* obj) {
    glUseProgram(shader_prog);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void*)(sizeof(f32) * 0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void*)(sizeof(f32) * 2));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void*)(sizeof(f32) * 4));

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

#else

static draw_rectb* rectb;
void test_desc_init(marena* arena, app_app* app, void* custon_data) {
    rectb = draw_rectb_create(arena, app->win, 64, 4);
    draw_rectb_finalize_textures(rectb);
}
void test_desc_destroy(void* custon_data) {
    draw_rectb_destroy(rectb);
}
void test_draw(app_app* app, void* obj) {
    memcpy(rectb->win_mat, app->win_mat, sizeof(app->win_mat));

    draw_rectb_push(rectb, (rect){ 50, 50, 50, 50 }, (vec4d){ 1, 1, 1, 1 });
    draw_rectb_flush(rectb);
}

#endif

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    obj_desc font_desc = {
        .name = STR("font"),
        .obj_size = sizeof(pres_font),

        .desc_init_func = test_desc_init,
        .desc_destroy_func = test_desc_destroy,

        .draw_func = test_draw
    };

    obj_reg_add_desc(arena, app, app->pres->obj_reg, &font_desc);
}
