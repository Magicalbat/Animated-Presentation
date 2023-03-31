#include "base/base_obj.h"

obj_register* obj_reg_create(marena* arena, u32 max_descs) {
    obj_register* obj_reg = CREATE_STRUCT(arena, obj_register);

    *obj_reg = (obj_register){
        .descs = CREATE_ZERO_ARRAY(arena, obj_desc, max_descs),
        .max_descs = max_descs,
        .num_descs = 0
    };
    
    return obj_reg;
}
void obj_reg_add_desc(marena* arena, app_app* app, obj_register* obj_reg, obj_desc* desc) {
    if (obj_reg->num_descs >= obj_reg->max_descs) {
        log_error("Ran out of desc slots");
        return;
    }

    memcpy(obj_reg->descs + obj_reg->num_descs, desc, sizeof(obj_desc));
    obj_reg->num_descs++;

    if (desc->desc_init_func != NULL)
        desc->desc_init_func(arena, app, desc->custom_data);
}
void obj_reg_destroy(obj_register* obj_reg) {
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (obj_reg->descs[i].desc_destroy_func != NULL)
            obj_reg->descs[i].desc_destroy_func(obj_reg->descs[i].custom_data);
    }
}
