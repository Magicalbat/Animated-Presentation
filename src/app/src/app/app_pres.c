#include "app/app_pres.h"

void app_pres_destroy(app_pres* pres) {
    for (app_slide_node* node = pres->first_slide; node != NULL; node = node->next) {
        app_objp_destroy(node->objs, pres->obj_reg);
    }
    
    obj_reg_destroy(pres->obj_reg);
    
    for (u32 i = 0; i < pres->num_plugins; i++) {
        os_lib_release(pres->plugins[i]);
    }
}

// TODO: make final version
void app_pres_draw(app_pres* pres, app_app* app) {
    app_objp_draw(pres->first_slide->objs, pres->obj_reg, app);
}
void app_pres_update(app_pres* pres, app_app* app, f32 delta) {
    app_animp_update(pres->first_slide->anims, app, delta);
    app_objp_update(pres->first_slide->objs, pres->obj_reg, app, delta);
}
