#include "plugin.h"

typedef struct {
    f64 x, y;
    vec2d scale;
    string8 source;
    vec4d col;

    u32 width;
    u32 height;
    u32 rect_img_id;
} pres_image;

void image_default(marena* arena, app_app* app, void* obj) {
    pres_image* img = (pres_image*)obj;

    *img = (pres_image){
        .width = 100,
        .height = 100,
        .scale = (vec2d){ 1, 1 },
        .col = (vec4d){ 1, 1, 1, 1 }
    };
}

void image_init(marena* arena, app_app* app, void* obj) {
    pres_image* img = (pres_image*)obj;

    vec2 dim = { 0 };
    img->rect_img_id = draw_rectb_load_tex(app->rectb, img->source, &dim);

    img->width = dim.x;
    img->height = dim.y;
}

void image_draw(app_app* app, void* obj) {
    pres_image* img = (pres_image*)obj;

    draw_rectb_push_ex(
        app->rectb,
        (rect){ (f32)img->x, (f32)img->y, img->width * img->scale.x, img->height * img->scale.y },
        img->col, img->rect_img_id, (rect){ 0 }
    );
}

void image_obj_init(marena* arena, app_app* app) {
    obj_desc desc = {
        .name = STR("image"),
        .obj_size = sizeof(pres_image),

        .default_func = image_default,
        .init_func = image_init,
        .draw_func = image_draw,

        .fields = {
            { STR("x"), FIELD_F64, offsetof(pres_image, x) },
            { STR("y"), FIELD_F64, offsetof(pres_image, y) },
            { STR("scale"), FIELD_VEC2D, offsetof(pres_image, scale) },
            { STR("source"), FIELD_STR8, offsetof(pres_image, source) },
            { STR("col"), FIELD_VEC4D, offsetof(pres_image, col) },
        }
    };

    obj_reg_add_desc(app->pres->obj_reg, &desc);
}
