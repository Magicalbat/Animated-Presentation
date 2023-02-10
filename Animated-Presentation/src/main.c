#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"
#include "draw/draw.h"
#include "draw/opengl_impl/gl_impl.h"

// TODO: use gles2 for wasm
// TODO: enable -Wall and fix warnings



// https://www.khronos.org/opengl/wiki/OpenGL_Error
/*void opengl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    log_level level = LOG_DEBUG;
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
}*/

#define WIN_SCALE 1

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init((log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });
    
    arena* perm_arena = arena_create(MiB(4));

    gfx_window* win = gfx_win_create(
        perm_arena,
        320 * WIN_SCALE, 180 * WIN_SCALE,
        STR8_LIT("Test window")
    );
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    log_infof("GL Vender: %s",   glGetString(GL_VENDOR));
    log_infof("GL Renderer: %s", glGetString(GL_RENDERER));
    log_infof("GL Version: %s",  glGetString(GL_VERSION));
    
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(opengl_message_callback, 0);

    draw_rectb* rectb = draw_rectb_create(perm_arena, win, 1024);
    //u32 monkey = draw_rectb_create_tex(perm_arena, rectb, STR8_LIT("monkey 1.png"));
    //u32 birds = draw_rectb_create_tex(perm_arena, rectb, STR8_LIT("kodim23.qoi"));

    draw_polygon* poly = draw_poly_create(perm_arena, win, 256);

    draw_cbezier* draw_bezier = draw_cbezier_create(perm_arena, win, 256);
    
    vec2 p[18];
    vec2_arr points = { .data=p, .size=18 };
    for (u32 i = 0; i < 18; i++) {
        float a = (f32)(i * 20) * (3.14159f / 180.0f);
        vec2 v = {
            .x = (sinf(a) * 5),
            .y = (cosf(a) * 5)
        };
        p[i] = v;
    }

    cbezier bezier = {
        (vec2){ 0, 0 },
        (vec2){ 0, 0.22 * 100 },
        (vec2){ 0.82 * 100, 0.34 * 100 },
        (vec2){ 100, 100 }
    };
    
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
    glViewport(0, 0, win->width, win->height);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    i32 cur_point = -1;

    while (!win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        glClear(GL_COLOR_BUFFER_BIT);

        /*for (u32 x = 0; x < 10; x++) {
            for (u32 y = 0; y < 10; y++) {
                draw_rectb_push(rectb, (rect){
                    (f32)x * 30, (f32)y * 30, 25.0f, 25.0f
                }, (vec3){
                    1.0f, 1.0f, 1.0f
                    //(f32)(x * 20) / 255.0f, (f32)(y * 20) / 255.0f, 1.0f,
                });
                //(x + y) % 2 == 0 ? monkey : birds, 
                //(rect){ 0, 0, 1, 1 });
            }
        }*/

        if (GFX_MOUSE_JUST_DOWN(win, GFX_MB_LEFT)) {
            for (u32 i = 0; i < 4; i++) {
                if (vec2_len(vec2_sub(win->mouse_pos, bezier.p[i])) <= 12) {
                    cur_point = i;
                    break;
                }
            }
        }
        if (GFX_MOUSE_JUST_UP(win, GFX_MB_LEFT)) {
            cur_point = -1;
        }

        if (cur_point != -1) {
            bezier.p[cur_point] = win->mouse_pos;
        }

        for (u32 i = 0; i < 4; i++) {
            draw_rectb_push(rectb, (rect){
                bezier.p[i].x - 6, bezier.p[i].y - 6, 12, 12
            }, (vec3){ 1, 1, 1});
        }

        draw_rectb_flush(rectb);

        draw_poly_conv_arr(poly, (vec3){ 0, 1, 1 }, win->mouse_pos, points);
        
        draw_cbezier_push_grad(draw_bezier, &bezier, 4,
            (vec3){ 0.0f, 1.0f, 0.0f }, (vec3){ 0.8f, 0.0f, 0.2f});
        draw_cbezier_flush(draw_bezier);

        gfx_win_swap_buffers(win);
        gfx_win_process_events(win);

        time_prev = time_now;
        os_sleep_milliseconds(16);
    }

    draw_cbezier_destroy(draw_bezier);
    draw_poly_destroy(poly);
    draw_rectb_destroy(rectb);

    gfx_win_destroy(win);
    
    arena_destroy(perm_arena);
    log_quit();
    os_main_quit();

    return 0;
}