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

    draw_rectb_t* batch = draw_rectb_create(perm_arena, 1024);

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);


    string8_list_t out_list = { 0 };
    str8_list_push(perm_arena, &out_list, str8_lit("num_rects, avg_delta\n"));

    u32 num_hund_rects = 0;
    u32 num_frames = 0;
    f32 avg_delta = 0;
    
    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    while (!win->info.should_close && num_hund_rects <= 100) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_process_events(win);

        glClear(GL_COLOR_BUFFER_BIT);

        /*for (u32 x = 0; x < 5; x++) {
            for (u32 y = 0; y < 5; y++) {
                draw_rectb_push(batch, (rect_t){
                    (f32)x / 2.5f - 1.0f,
                    (f32)y / 2.5f - 1.0f,
                    0.2f, 0.2f
                });
            }
        }*/

        for (int y = 0; y < num_hund_rects; y++) {
            for (int x = 0; x < 100; x++) {
                draw_rectb_push(batch, (rect_t){
                    0.0, 0.0,
                    //(f32)x / 50.0f - 1.0f,
                    //1.0f - (f32)y / (num_hund_rects * 0.5),
                    0.1f, 0.1f
                });
            }
        }

        num_frames++;
        avg_delta += delta;
        if (num_frames >= 120) {
            avg_delta /= 120;

            if (num_hund_rects) {
                string8_t line = str8_pushf(perm_arena, "%lu, %f\n", num_hund_rects * 100, avg_delta);
                str8_list_push(perm_arena, &out_list, line);
            }
            
            num_hund_rects += 1;
            if (num_hund_rects % 10 == 0)
                printf("%u\n", num_hund_rects);
            avg_delta = 0;
        }
        
        //vec2_t offset = { sinf(theta) * 0.1f, cosf(theta) * 0.1f };
        //for (int x = 0; x < 16; x++) {
        //    for (int y = 0; y < 9; y++) {
        //        draw_rectb_push(batch, (rect_t){
        //            (f32)x / 8.0f - 1.0f + offset.x,
        //            (f32)y / 4.5f - 1.0f + offset.y,
        //            0.1f, 0.1f
        //        });
        //    }
        //}
        
        draw_rectb_flush(batch);

        gfx_win_swap_buffers(win);

        //os_sleep_milliseconds(MAX(0, (0.0167f - delta) * 1000));
        
        time_prev = time_now;
    }

    os_file_write(str8_lit("batching.csv"), out_list);

    draw_rectb_destroy(batch);

    gfx_win_destroy(win);

    arena_destroy(perm_arena);

    os_main_quit();

    return 0;
}
