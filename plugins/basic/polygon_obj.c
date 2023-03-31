#include "plugin.h"

typedef struct {
    f64 x;
    f64 y;
    struct {
        u64 size;
        vec2d* data;
    } points;
    vec4d col;
} pres_poly;

void poly_default(marena* arena, app_app* app, void* obj) {
    pres_poly* poly = (pres_poly*)obj;

    *poly = (pres_poly){
        .col = (vec4d){ 1, 1, 1, 1 }
    };
}

void poly_draw(app_app* app, void* obj) {
    pres_poly* poly = (pres_poly*)obj;

    vec2d_arr points = *(vec2d_arr*)(&poly->points);

    draw_poly_conv_arr(
        app->poly,
        poly->col,
        (vec2d){ poly->x, poly->y },
        points
    );
}

void polygon_obj_init(marena* arena, app_app* app) {
    obj_desc desc = {
        .name = STR("polygon"),
        .obj_size = sizeof(pres_poly),

        .default_func = poly_default,
        .draw_func = poly_draw,

        .fields = {
            { STR("x"     ), FIELD_F64,       offsetof(pres_poly, x     ) },
            { STR("y"     ), FIELD_F64,       offsetof(pres_poly, y     ) },
            { STR("points"), FIELD_VEC2D_ARR, offsetof(pres_poly, points) },
            { STR("col"   ), FIELD_VEC4D,     offsetof(pres_poly, col   ) },
        }
    };

    obj_reg_add_desc(arena, app, app->pres->obj_reg, &desc);
}
