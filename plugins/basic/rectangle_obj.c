#include "plugin.h"

typedef struct {
    f64 x, y;
    f64 w, h;
    b32 fill;
    vec4d fill_col;
    b32 outline;
    vec4d outline_col;
    f64 outline_width;
} pres_rect;

void rect_default(marena* arena, app_app* app, void* obj) {
    pres_rect* r = (pres_rect*)obj;

    *r = (pres_rect){ 
        .fill = true,
        .fill_col = (vec4d){ 1, 1, 1, 1 },
        .outline_width = 2,
        .outline_col = (vec4d){ 1, 1, 1, 1 }
    };
}

void rect_draw(app_app* app, void* obj) {
    pres_rect* r = (pres_rect*)obj;

    if (r->fill) {
        f64 w = r->outline ? r->outline_width : 0;
        
        draw_rectb_push(app->rectb, (rect){
            (f32)r->x + w, (f32)r->y + w, (f32)r->w - w - w, (f32)r->h - w - w
        }, r->fill_col);
    }

    if (r->outline) {
        f64 w = r->outline_width;
        
        draw_rectb_push(app->rectb, (rect){
            (f32)r->x, (f32)r->y, (f32)w, (f32)r->h
        }, r->outline_col);
        draw_rectb_push(app->rectb, (rect){
            (f32)r->x + w, (f32)r->y, (f32)r->w - w - w, (f32)w
        }, r->outline_col);
        draw_rectb_push(app->rectb, (rect){
            (f32)r->x + r->w - w, (f32)r->y, (f32)w, (f32)r->h
        }, r->outline_col);
        draw_rectb_push(app->rectb, (rect){
            (f32)r->x + w, (f32)r->y + r->h - w, (f32)r->w - w - w, (f32)w
        }, r->outline_col);
    }
}

void rectangle_obj_init(marena* arena, app_app* app) {
    obj_desc desc = {
        .name = STR("rectangle"),
        .obj_size = sizeof(pres_rect),

        .default_func = rect_default,
        .draw_func = rect_draw,

        .fields = {
            { STR("x"), FIELD_F64, offsetof(pres_rect, x) },
            { STR("y"), FIELD_F64, offsetof(pres_rect, y) },
            { STR("w"), FIELD_F64, offsetof(pres_rect, w) },
            { STR("h"), FIELD_F64, offsetof(pres_rect, h) },
            { STR("fill"), FIELD_BOOL32, offsetof(pres_rect, fill) },
            { STR("fill_col"), FIELD_VEC4D, offsetof(pres_rect, fill_col) },
            { STR("col"), FIELD_VEC4D, offsetof(pres_rect, fill_col) },
            { STR("outline"), FIELD_BOOL32, offsetof(pres_rect, outline) },
            { STR("outline_col"), FIELD_VEC4D, offsetof(pres_rect, outline_col) },
            { STR("outline_width"), FIELD_F64, offsetof(pres_rect, outline_width) },
        }
    };

    obj_reg_add_desc(app->pres->obj_reg, &desc);
}
