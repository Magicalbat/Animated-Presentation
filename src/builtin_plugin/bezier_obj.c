#include "plugin.h"

typedef struct {
    vec2d p0;
    vec2d p1;
    vec2d p2;
    vec2d p3;
    f64 width;
    vec4d col;
    b32 gradient;
    vec4d start_col;
    vec4d end_col;
} pres_bezier;

void bezier_default(marena* arena, ap_app* app, void* obj){
    pres_bezier* bez = (pres_bezier*)obj;

    *bez = (pres_bezier){
        .width = 4,
        .col = (vec4d){ 1, 1, 1, 1 }
    };
}

void bezier_draw(ap_app* app, void* obj) {
    pres_bezier* bez = (pres_bezier*)obj;

    cbezier draw_bez = {
        .p0 = (vec2){ (f32)bez->p0.x, (f32)bez->p0.y },
        .p1 = (vec2){ (f32)bez->p1.x, (f32)bez->p1.y },
        .p2 = (vec2){ (f32)bez->p2.x, (f32)bez->p2.y },
        .p3 = (vec2){ (f32)bez->p3.x, (f32)bez->p3.y }
    };

    if (bez->gradient) {
        draw_cbezier_push_grad(
            app->cbezier,
            &draw_bez,
            (u32)bez->width,
            bez->start_col,
            bez->end_col
        );
    } else {
        draw_cbezier_push(
            app->cbezier,
            &draw_bez,
            (u32)bez->width,
            bez->col
        );
    }
}

void bezier_obj_init(marena* arena, ap_app* app) {
    obj_desc desc = {
        .name = STR("bezier"),
        .obj_size = sizeof(pres_bezier),

        .default_func = bezier_default,
        .draw_func = bezier_draw,

        .fields = {
            { STR("p0"), FIELD_VEC2D, offsetof(pres_bezier, p0) },
            { STR("p1"), FIELD_VEC2D, offsetof(pres_bezier, p1) },
            { STR("p2"), FIELD_VEC2D, offsetof(pres_bezier, p2) },
            { STR("p3"), FIELD_VEC2D, offsetof(pres_bezier, p3) },
            { STR("width"), FIELD_F64, offsetof(pres_bezier, width) },
            { STR("col"), FIELD_VEC4D, offsetof(pres_bezier, col) },
            { STR("gradient"), FIELD_BOOL32, offsetof(pres_bezier, gradient) },
            { STR("start_col"), FIELD_VEC4D, offsetof(pres_bezier, start_col) },
            { STR("end_col"), FIELD_VEC4D, offsetof(pres_bezier, end_col) },
        }
    };

    obj_reg_add_desc(app->pres->obj_reg, &desc);
}
