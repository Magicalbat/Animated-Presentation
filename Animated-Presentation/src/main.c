#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"
#include "gfx/draw/draw.h"

// TODO: Figure out my prefered error handling method

// https://www.khronos.org/opengl/wiki/OpenGL_Error
void opengl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
             type, severity, message);
}

#define WIN_SCALE 1

int main(int argc, char** argv) {
    os_main_init(argc, argv);
    
    arena_t* perm_arena = arena_create(KB(64));

    gfx_window_t* win = gfx_win_create(perm_arena, 320, 180, str8_lit("Test window"));
    gfx_win_make_current(win);
    opengl_load_functions(win);

    printf("GL Vender: %s\n",   glGetString(GL_VENDOR));
    printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("GL Version: %s\n",  glGetString(GL_VERSION));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_message_callback, 0);

    draw_rectb_t* batch = draw_rectb_create(perm_arena, win, 1024);

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();
    
    f32 theta = 0;

    while (!win->info.should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        theta += delta * 2.0f;

        gfx_win_process_events(win);

        glClear(GL_COLOR_BUFFER_BIT);

        vec2_t offset = { sinf(-theta) * 10.0f, cosf(theta) * 10.0f };
        f32 b = (sinf(theta) * 0.5f + 0.5f);
        for (int x = 0; x < 20; x++) {
            for (int y = 0; y < 11; y++) {
                draw_rectb_push(batch, (rect_t){
                    (f32)x * 16 + offset.x,
                    (f32)y * 16 + offset.y,
                    8.0f, 8.0f
                }, (vec3_t){
                    (f32)x / 16, 
                    (f32)y / 16, 
                    b
                });
            }
        }
        
        draw_rectb_flush(batch);

        gfx_win_swap_buffers(win);

        os_sleep_milliseconds(MAX(0, (0.0167f - delta) * 1000));
        
        time_prev = time_now;
    }

    draw_rectb_destroy(batch);

    gfx_win_destroy(win);

    arena_destroy(perm_arena);

    os_main_quit();

    return 0;
}
