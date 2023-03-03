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

    FIELD_COUNT
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

static const u32 field_sizes[FIELD_COUNT] = {
    0,
    sizeof(f64),
    sizeof(string8),
    sizeof(b32),
    sizeof(vec2),
    sizeof(vec3),
    sizeof(vec4),
    sizeof(struct { void* data; u64 size; })
};

typedef struct ap_app ap_app;

typedef void (desc_init_func)(void* custom_data);
typedef void (desc_destroy_func)(void* custom_data);

typedef void (obj_init_func)(void* obj);
typedef void (obj_destroy_func)(void* obj);
typedef void (obj_draw_func)(ap_app* app, void* obj);
typedef void (obj_update_func)(f32 delta, void* obj);

#define DESC_MAX_FIELDS 32
typedef struct {
    string8 name;
    u32 obj_size;

    void* custom_data;
    desc_init_func* desc_init_func;
    desc_destroy_func* desc_destroy_func;

    obj_init_func* init_func;
    obj_destroy_func* destroy_func;
    obj_draw_func* draw_func;
    obj_update_func* update_func;

    string8 field_names[DESC_MAX_FIELDS];
    field_type field_types[DESC_MAX_FIELDS];
    u32 field_offsets[DESC_MAX_FIELDS];
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
    u32 desc_index;
    void* obj;
} obj_ref;

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
    struct slide_node* prev;

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

struct ap_app {
    gfx_window* win;

    draw_rectb* rectb;
    draw_cbezier* cbezier;
    draw_polygon* poly;

    pres* pres;
};

obj_register* obj_reg_create(marena* arena, u32 max_descs);
void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc);
void obj_reg_destroy(obj_register* obj_reg);

obj_pool* obj_pool_create(marena* arena, obj_register* obj_reg, u32 max_objs);
obj_ref obj_pool_add(obj_pool* pool, obj_register* obj_reg, string8 name);
void obj_pool_draw(obj_pool* pool, obj_register* obj_reg, ap_app* app);
void obj_pool_update(obj_pool* pool, obj_register* obj_reg, f32 delta);
void obj_pool_destroy(obj_pool* pool, obj_register* obj_reg);

void obj_ref_set(obj_ref ref, obj_register* obj_reg, string8 prop, void* data);

ap_app* app_create(marena* arena, pres* pres, u32 win_width, u32 win_height);
void app_run(marena* arena, ap_app* app);
void app_destroy(ap_app* app);

void slide_draw(slide_node* slide, ap_app* app);
void slide_update(slide_node* slide, f32 delta);

typedef struct {
    f32 x, y, w, h;
    vec3 col;
} pres_rect;

void rect_init_plugin(marena* arena, obj_register* obj_reg);

#define WIN_SCALE 1
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

    ap_app* app = app_create(perm_arena, NULL, WIDTH, HEIGHT);
    app_run(perm_arena, app);
    app_destroy(app);

    /*gfx_window* win = gfx_win_create(
        perm_arena,
        WIDTH, HEIGHT,
        STR8_LIT("Test window")
    );
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    gfx_win_alpha_blend(win, true);
    gfx_win_clear_color(win, (vec3){ 0.5f, 0.6f, 0.7f });

    obj_register* obj_reg = obj_reg_create(perm_arena, 16);
    rect_init_plugin(perm_arena, obj_reg);
    obj_reg_destroy(obj_reg);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    while (!win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_clear(win);

        gfx_win_swap_buffers(win);
        gfx_win_process_events(win);

        time_prev = time_now;
        u64 sleep_time_ms = MAX(0, 16 - (i64)(delta * 1000));
        os_sleep_milliseconds(sleep_time_ms);
    }

    gfx_win_destroy(win);*/
    
    marena_destroy(perm_arena);
    log_quit();
    
    return 0;
}

obj_register* obj_reg_create(marena* arena, u32 max_descs) {
    obj_register* obj_reg = CREATE_STRUCT(arena, obj_register);

    *obj_reg = (obj_register){
        .descs = CREATE_ZERO_ARRAY(arena, obj_desc, max_descs),
        .max_descs = max_descs,
        .num_descs = 0
    };
    
    return obj_reg;
}
void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc) {
    if (obj_reg->num_descs >= obj_reg->max_descs) {
        log_error("Ran out of desc slots");
        return;
    }

    memcpy(obj_reg->descs + obj_reg->num_descs, desc, sizeof(obj_desc));
    obj_reg->num_descs++;

    if (desc->desc_init_func != NULL)
        desc->desc_init_func(desc->custom_data);
}
void obj_reg_destroy(obj_register* obj_reg) {
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (obj_reg->descs[i].desc_destroy_func != NULL)
            obj_reg->descs[i].desc_destroy_func(obj_reg->descs[i].custom_data);
    }
}

obj_pool* obj_pool_create(marena* arena, obj_register* obj_reg, u32 max_objs) {
    obj_pool* pool = CREATE_STRUCT(arena, obj_pool);

    *pool = (obj_pool){
        .max_objs = max_objs,
        .num_objs = CREATE_ZERO_ARRAY(arena, u32, obj_reg->num_descs),
        .objs = CREATE_ZERO_ARRAY(arena, void*, obj_reg->num_descs)
    };

    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        pool->objs[i] = (void*)marena_push_zero(arena, obj_reg->descs[i].obj_size);
    }

    return pool;
}
obj_ref obj_pool_add(obj_pool* pool, obj_register* obj_reg, string8 name) {
    i64 desc_index = -1;
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (str8_equals(name, obj_reg->descs[i].name)){ 
            desc_index = i;
            break;
        }
    }
    if (desc_index == -1) {
        log_errorf("Failed to locate object %.*s in register", (int)name.size, (char*)name.str);
        return (obj_ref){ 0 };
    }


    u32 index = pool->num_objs[desc_index];
    void* obj = (void*)((u8*)pool->objs[index] + obj_reg->descs[desc_index].obj_size * index);

    obj_ref out = {
        .desc_index = desc_index,
        //.index = index,
        .obj = obj
    };

    pool->num_objs[desc_index]++;

    if (obj_reg->descs[desc_index].init_func != NULL) {
        obj_reg->descs[desc_index].init_func(obj);
    }

    return out;
}

// TODO: use macro for these?
void obj_pool_draw(obj_pool* pool, obj_register* obj_reg, ap_app* app) {
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (obj_reg->descs[i].draw_func == NULL)
            continue;

        u32 obj_size = obj_reg->descs[i].obj_size;
        void* obj = pool->objs[i];

        for (u32 j = 0; j < pool->num_objs[i]; j++) {
            obj_reg->descs[i].draw_func(app, obj);
            obj = (void*)((u8*)obj + obj_size);
        }
    }
}
void obj_pool_update(obj_pool* pool, obj_register* obj_reg, f32 delta) {
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (obj_reg->descs[i].update_func == NULL)
            continue;

        u32 obj_size = obj_reg->descs[i].obj_size;
        void* obj = pool->objs[i];

        for (u32 j = 0; j < pool->num_objs[i]; j++) {
            obj_reg->descs[i].update_func(delta, obj);
            obj = (void*)((u8*)obj + obj_size);
        }
    }
}
void obj_pool_destroy(obj_pool* pool, obj_register* obj_reg) {
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (obj_reg->descs[i].destroy_func == NULL)
            continue;

        u32 obj_size = obj_reg->descs[i].obj_size;
        void* obj = pool->objs[i];

        for (u32 j = 0; j < pool->num_objs[i]; j++) {
            obj_reg->descs[i].destroy_func(obj);
            obj = (void*)((u8*)obj + obj_size);
        }
    }
}

void obj_ref_set(obj_ref ref, obj_register* obj_reg, string8 prop, void* data) {
    obj_desc* desc = &obj_reg->descs[ref.desc_index];

    i64 field_index = -1;
    for (u32 i = 0; i < DESC_MAX_FIELDS; i++) {
        if (desc->field_types[i] == FIELD_NULL) break;

        if (str8_equals(prop, desc->field_names[i])) {
            field_index = i;
            break;
        }
    }

    if (field_index == -1) {
        log_errorf("Failed to find field \"%.*s\"", (int)prop.size, (char*)prop.str);
        return;
    }

    u32 field_size = field_sizes[desc->field_types[field_index]];
    void* field_ptr = (void*)((u8*)ref.obj + desc->field_offsets[field_index]);

    memcpy(field_ptr, data, field_size);
}

ap_app* app_create(marena* arena, pres* pres, u32 win_width, u32 win_height) {
    ap_app* app = CREATE_STRUCT(arena, ap_app);

    gfx_window* win = gfx_win_create(arena, win_width, win_height, STR8_LIT("Animated Presentation"));
    
    gfx_win_make_current(win);
    opengl_load_functions(win);

    app->win = win;
    app->rectb = draw_rectb_create(arena, win, 1024, 32);
    app->cbezier = draw_cbezier_create(arena, win, 1024),
    app->poly = draw_poly_create(arena, win, 256),
    app->pres = NULL;

    return app;
} 
void app_run(marena* arena, ap_app* app) {
    draw_rectb_finalize_textures(app->rectb);

    gfx_win_alpha_blend(app->win, true);
    gfx_win_clear_color(app->win, (vec3){ 0.5f, 0.6f, 0.7f });

    obj_register* obj_reg = obj_reg_create(arena, 16);
    rect_init_plugin(arena, obj_reg);

    u64 time_prev = os_now_microseconds();
    while (!app->win->should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_clear(app->win);

        gfx_win_swap_buffers(app->win);
        gfx_win_process_events(app->win);

        time_prev = time_now;
        u64 sleep_time_ms = MAX(0, 16 - (i64)(delta * 1000));
        os_sleep_milliseconds(sleep_time_ms);
    }

    obj_reg_destroy(obj_reg);
}
void app_destroy(ap_app* app) {
    draw_rectb_destroy(app->rectb);
    draw_cbezier_destroy(app->cbezier);
    draw_poly_destroy(app->poly);

    gfx_win_destroy(app->win);
}

void slide_draw(slide_node* slide, ap_app* app) { }
void slide_update(slide_node* slide, f32 delta) { }

typedef struct {
    char a, b, c;
} rect_plug_data;

void rect_desc_init(void* custom_data) {
    rect_plug_data* plug_data = (rect_plug_data*)custom_data;
    
    plug_data->a = 'x';
    plug_data->b = 'y';
    plug_data->c = 'z';
}
void rect_desc_destroy(void* custom_data) {
    rect_plug_data* plug_data = (rect_plug_data*)custom_data;

    log_debugf("rect plug data: %c %c %c", plug_data->a, plug_data->b, plug_data->c);
}

void rect_init(void* obj) {
    pres_rect* r = (pres_rect*)obj;
    *r = (pres_rect){
        .w = 50,
        .h = 50,
        .col = (vec3){ 0, 1, 0 }
    };
    log_debug("rect obj init");
}
void rect_destroy(void* obj) {
    pres_rect* r = (pres_rect*)obj;
    log_debug("rect obj destroy");
}
void rect_draw(ap_app* app, void* obj) {
    pres_rect* r = (pres_rect*)obj;

    draw_rectb_push(app->rectb, (rect){
        r->x, r->y, r->w, r->h
    }, r->col);
}
void rect_update(f32 delta, void* obj) {
    pres_rect* r = (pres_rect*)obj;

    r->x += 4.0f * delta;
    r->y += 3.0f * delta;
}

void rect_init_plugin(marena* arena, obj_register* obj_reg) {
    rect_plug_data* data = CREATE_ZERO_STRUCT(arena, rect_plug_data);

    obj_reg_add_desc(obj_reg, &(obj_desc){
        .obj_size = sizeof(pres_rect),
        .custom_data = (void*)data,

        .desc_init_func = rect_desc_init,
        .desc_destroy_func = rect_desc_destroy,

        .init_func = rect_init,
        .destroy_func = rect_destroy,
        .draw_func = rect_draw,
        .update_func = rect_update,

        .field_names = {
            STR8_LIT("x"),
            STR8_LIT("y"),
            STR8_LIT("z"),
            STR8_LIT("w"),
            STR8_LIT("col"),
        },
        .field_types = {
            FIELD_NUM,
            FIELD_NUM,
            FIELD_NUM,
            FIELD_NUM,
            FIELD_VEC3
        },
        .field_offsets = {
            offsetof(pres_rect, x),
            offsetof(pres_rect, y),
            offsetof(pres_rect, w),
            offsetof(pres_rect, h),
            offsetof(pres_rect, col),
        }
    });
}
