#include "ap_core.h"
#include "app/app.h"

typedef struct {
    f64 x, y;
    f64 w, h;
    vec4d col;
    b32 visible;
    struct {
        u64 size;
        vec3d* data;
    } points;
} pres_rect;

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
        .visible = true,
        .col = (vec4d){ 1, 1, 1, 1 }
    };
    log_debug("rect obj init");
}
void rect_destroy(void* obj) {
    pres_rect* r = (pres_rect*)obj;

    printf("[ ");
    for (u64 i = 0; i < r->points.size; i++) {
        printf("{ %.1f, %.1f, %.1f }, ", r->points.data[i].x, r->points.data[i].y, r->points.data[i].z);
    }
    if (r->points.size != 0)
        printf("\b\b ");
    printf("]\n");
    
    log_debug("rect obj destroy");
}
void rect_draw(ap_app* app, void* obj) {
    pres_rect* r = (pres_rect*)obj;
    
    if (!r->visible) return;
    
    draw_rectb_push(app->rectb, (rect){
        (f32)r->x, (f32)r->y, (f32)r->w, (f32)r->h
    }, r->col);
}
void rect_update(f32 delta, void* obj) {
    pres_rect* r = (pres_rect*)obj;

    r->x += 8.0f * delta;
    r->y += 6.0f * delta;
    r->col.w -= 0.1 * delta;
}

AP_EXPORT void plugin_init(marena* arena, ap_app* app) {
    rect_plug_data* data = CREATE_ZERO_STRUCT(arena, rect_plug_data);

    obj_reg_add_desc(app->pres->obj_reg, &(obj_desc){
        .name = STR8_LIT("rectangle"),
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
            STR8_LIT("w"),
            STR8_LIT("h"),
            STR8_LIT("col"),
            STR8_LIT("visible"),
            STR8_LIT("points"),
        },
        .field_types = {
            FIELD_F64,
            FIELD_F64,
            FIELD_F64,
            FIELD_F64,
            FIELD_VEC4D,
            FIELD_BOOL32,
            FIELD_VEC3D_ARR,
        },
        .field_offsets = {
            offsetof(pres_rect, x),
            offsetof(pres_rect, y),
            offsetof(pres_rect, w),
            offsetof(pres_rect, h),
            offsetof(pres_rect, col),
            offsetof(pres_rect, visible),
            offsetof(pres_rect, points),
        }
    });
}
