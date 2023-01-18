#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"
#include "gfx/draw/draw.h"
#include "gfx/draw/opengl_impl/gl_impl.h"

#include "parse/parse.h"

#include <dlfcn.h>

typedef int (*math_func)(int, int);
typedef const char* (*vers_func)();

// https://www.khronos.org/opengl/wiki/OpenGL_Error
void opengl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    log_level_t level = LOG_DEBUG;
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            level = LOG_DEBUG;
            break;
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_MEDIUM:
            level = LOG_WARN;
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            level = LOG_ERROR;
            break;
        default: break;
    }
    log_msgf(level, "GL CALLBACK - type = 0x%x, severity = 0x%x, message = %s",
        type, severity, message);
}

#define WIN_SCALE 1

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init((log_desc_t){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });

    arena_t* perm_arena = arena_create(MiB(16));

    gfx_window_t* win = gfx_win_create(
        perm_arena,
        320 * WIN_SCALE, 180 * WIN_SCALE,
        STR8_LIT("Test window")
    );
    gfx_win_make_current(win);
    opengl_load_functions(win);

    //log_infof("GL Vender: %s",   glGetString(GL_VENDOR));
    //log_infof("GL Renderer: %s", glGetString(GL_RENDERER));
    //log_infof("GL Version: %s",  glGetString(GL_VERSION));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_message_callback, 0);

    os_library_t lib = os_lib_load(STR8_LIT("./test_lib.so"));

    math_func testlib_add = (math_func)os_lib_func(lib, STR8_LIT("testlib_add"));
    math_func testlib_sub = (math_func)os_lib_func(lib, STR8_LIT("testlib_sub"));
    //vers_func testlib_version = (vers_func)dlsym(handle, "testlib_version");

    log_debugf("%d %d", testlib_add(1, 2), testlib_sub(5, 3));
    //log_debugf("%d %d %s", testlib_add(1, 2), testlib_sub(5, 3), testlib_version());

    os_lib_release(lib);

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    const char* vertex_shader = ""
        "#version 330 core\n"
        "layout(location = 0) in vec2 a_pos;"
        "layout(location = 1) in vec2 a_uv;"
        "out vec2 uv;"
        "void main() {"
        "    gl_Position = vec4(a_pos, 0., 1.);"
        "    uv = a_uv;"
        "}";
    
    const char* frag_shader = ""
        "#version 330 core\n"
        "in vec2 uv;"
        "uniform sampler2D texture1;"
        "void main() {"
        "    gl_FragColor = texture(texture1, uv);"
        "}";

    u32 shader_program = gl_impl_create_shader_program(vertex_shader, frag_shader);
    
    f32 vertices[] = {
        // positions         // texture coords
         0.75f,  0.75f,  1.0f, 0.0f,
         0.75f, -0.75f,  1.0f, 1.0f,
        -0.75f, -0.75f,  0.0f, 1.0f,
        -0.75f,  0.75f,  0.0f, 0.0f 
    };
    u32 indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    u32 vert_array;
    glGenVertexArrays(1, &vert_array);
    glBindVertexArray(vert_array);
    
    u32 vert_buffer = gl_impl_create_buffer(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    u32 index_buffer = gl_impl_create_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    string8_t img_file = os_file_read(perm_arena, STR8_LIT("kodim23.qoi"));
    image_t img = { 0 };
    if (img_file.size) {
        img = parse_qoi(perm_arena, img_file);
    }

    u32 color_type = img.channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, color_type, img.width, img.height, 0, color_type, GL_UNSIGNED_BYTE, img.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();
    
    while (!win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_process_events(win);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vert_array);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        gfx_win_swap_buffers(win);

        time_prev = time_now;
    }

    glDeleteVertexArrays(1, &vert_array);
    glDeleteBuffers(1, &vert_buffer);
    glDeleteBuffers(1, &index_buffer);

    gfx_win_destroy(win);

    arena_destroy(perm_arena);

    log_quit();

    os_main_quit();

    return 0;
}
