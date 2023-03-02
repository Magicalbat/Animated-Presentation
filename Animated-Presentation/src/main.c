#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"
#include "draw/draw.h"
#include "draw/opengl_impl/gl_impl.h"

typedef enum {
    FIELD_NULL,
    FIELD_NUM,
    FIELD_STR,
    FIELD_BOOL,
    FIELD_VEC2,
    FIELD_VEC3,
    FIELD_VEC4,
    FIELD_ARR,
} field_type;

typedef struct field_val {
    field_type type;

    union {
        struct { int _unused; } null;
        f64 num;
        string8 str;
        b32 boolean;
        vec2 vec2;
        vec3 vec3;
        vec4 vec4;
        struct {
            struct field_val* data;
            u64 size;
        } arr;
    } val;
} field_val;

typedef void (desc_init_func)(void* custom_data);
typedef void (desc_destroy_func)(void* custom_data);

typedef void (obj_init_func)(void* obj);
typedef void (obj_destroy_func)(void* obj);
typedef void (obj_draw_func)(void* obj);
typedef void (obj_update_func)(void* obj);

#define MAX_FIELDS 32
typedef struct {
    u32 obj_size;

    void* custom_data;
    desc_init_func* desc_init_func;
    desc_destroy_func* desc_destroy_func;

    obj_init_func* init_func;
    obj_destroy_func* destroy_func;
    obj_draw_func* draw_func;
    obj_update_func* update_func;

    string8 field_names[MAX_FIELDS];
    field_type field_types[MAX_FIELDS];
    u32 field_offsets[MAX_FIELDS];
} obj_desc;

typedef struct {
    obj_desc* descs;
    u32 max_descs;
    u32 num_descs;
} obj_register;

typedef struct {
    u32 max_objs;
    u32* num_objs;
    void** objs;
} obj_pool;

typedef struct {
    field_type type;
    void* obj_field;

    u32 num_keys;
    field_val* keys;
    f32* times;

    f32 cur_time;
} anim;

typedef struct {
    anim* anims;
    u32 num_anims;
} anim_pool;

typedef struct slide_node {
    struct slide_node* next;
    obj_pool objs;
    anim_pool anims;
} slide_node;

typedef struct {
    obj_register* obj_reg;

    slide_node* first;
    slide_node* last;
    u32 num_slides;

    u32 cur_slide;
} pres;

typedef struct {
    gfx_window* win;

    draw_rectb* rectb;
    draw_cbezier* cbezier;
    draw_polygon* poly;

    pres* pres;
} ap_app;

obj_register* obj_reg_create(marena* arena);
void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc);
void obj_reg_destroy(obj_register* obj_reg);

ap_app* app_create(marena* arena, pres* pres);
void app_run(ap_app* app);
void app_destroy(ap_app* app);

void slide_draw(slide_node* slide, ap_app* app);
void slide_update(slide_node* slide, f32 delta);

#define WIN_SCALE 1.5
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    log_init((log_desc){ 
        .log_time = LOG_NO,
        .log_file = { 0, 0, LOG_NO, LOG_NO }
    });
    
    marena* perm_arena = marena_create(&(marena_desc){
        .desired_max_size = MiB(4),
        .desired_block_size = KiB(64)
    });

    gfx_window* win = gfx_win_create(
        perm_arena,
        WIDTH, HEIGHT,
        STR8_LIT("Test window")
    );
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    log_infof("GL Vender: %s",   glGetString(GL_VENDOR));
    log_infof("GL Renderer: %s", glGetString(GL_RENDERER));
    log_infof("GL Version: %s",  glGetString(GL_VERSION));

    gfx_win_alpha_blend(win, true);
    gfx_win_clear_color(win, (vec3){ 0.5f, 0.6f, 0.7f });

    draw_rectb* rectb = draw_rectb_create(perm_arena, win, 1024, 16);
    draw_rectb_finalize_textures(rectb);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    while (!win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_clear(win);

        draw_rectb_flush(rectb);

        gfx_win_swap_buffers(win);
        gfx_win_process_events(win);

        time_prev = time_now;
        os_sleep_milliseconds(16);
    }

    draw_rectb_destroy(rectb);

    gfx_win_destroy(win);
    
    marena_destroy(perm_arena);
    log_quit();
    
    return 0;
}

obj_register* obj_reg_create(marena* arena) { return NULL; }
void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc) { }
void obj_reg_destroy(obj_register* obj_reg) { }

ap_app* app_create(marena* arena, pres* pres) { return NULL; } 
void app_run(ap_app* app) { } 
void app_destroy(ap_app* app) { }

void slide_draw(slide_node* slide, ap_app* app) { }
void slide_update(slide_node* slide, f32 delta) { }