#ifdef AP_OPENGL

#include "base/base.h"
#include "gfx/gfx.h"

static const char* vert_source;
static const char* frag_source;

draw_cbezier* draw_cbezier_create(arena* arena, u32 capacity) {
    draw_cbezier* draw_cb = CREATE_ZERO_STRUCT(arena, draw_cb, draw_cbezier);

    
}
void draw_cbezier_destroy(draw_cbezier* draw_cb) { }

void draw_cbezier_push(draw_cbezier* draw_cb, vec3 col);
void draw_cbezier_push_grad(draw_cbeier* draw_cb, vec3 start_col, vec3 end_col);
void draw_cbezier_flush(draw_cbezier* draw_cb);

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

static const char* frag_source = ""
    "#version 330 core\n"
    "in vec4 col;"
    "void main() {"
    "    gl_FragColor = col;"
    "\n}";

#endif // AP_OPENGL