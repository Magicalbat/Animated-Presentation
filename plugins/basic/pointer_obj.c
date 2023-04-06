#include "plugin.h"

typedef struct {
    f64 w, h;
    vec4d col;
    string8 source;

    u32 file_reg_id;
    u32 img_id;
} pres_pointer;

void pointer_default(marena* arena, app_app* app, void* obj) {
    pres_pointer* pointer = (pres_pointer*)obj;

    *pointer = (pres_pointer) {
        .col = (vec4d){ 1, 1, 1, 1 },
        .w = 10, .h = 10,
        .img_id = 0
    };
}

void pointer_init(marena* arena, app_app* app, void* obj) {
    pres_pointer* pointer = (pres_pointer*)obj;

    if (pointer->source.size) {
        pointer->file_reg_id = str8_reg_push(app->temp.arena, &app->temp.file_reg, pointer->source);
    }
}

void pointer_file(marena* arena, app_app* app, void* obj) {
    pres_pointer* pointer = (pres_pointer*)obj;

    if (pointer->source.size) {
        pointer->img_id = draw_rectb_create_tex(
            app->rectb,
            str8_reg_get(&app->temp.file_reg, pointer->file_reg_id),
            NULL
        );
    }
}

void pointer_draw(app_app* app, void* obj) {
    pres_pointer* pointer = (pres_pointer*)obj;

    draw_rectb_push_ex(
        app->rectb,
        (rect){ app->mouse_pos.x - pointer->w * 0.5, app->mouse_pos.y - pointer->h * 0.5, pointer->w, pointer->h },
        pointer->col,
        pointer->img_id,
        (rect){ 0 }
    );
}

void pointer_obj_init(marena* arena, app_app* app) {
    obj_desc desc = {
        .name = STR("pointer"),
        .obj_size = sizeof(pres_pointer),
        
        .default_func = pointer_default,
        .init_func = pointer_init,
        .file_func = pointer_file,
        .draw_func = pointer_draw,

        .fields = {
            { STR("w"     ), FIELD_F64,   offsetof(pres_pointer, w     ) },
            { STR("h"     ), FIELD_F64,   offsetof(pres_pointer, h     ) },
            { STR("col"   ), FIELD_VEC4D, offsetof(pres_pointer, col   ) },
            { STR("source"), FIELD_STR8,  offsetof(pres_pointer, source) },
        }
    };

    obj_reg_add_desc(arena, app, app->pres->obj_reg, &desc);
}
