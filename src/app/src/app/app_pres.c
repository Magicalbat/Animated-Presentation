#include "app/app_pres.h"

void pres_delete(apres* pres) {
    for (slide_node* node = pres->first_slide; node != NULL; node = node->next) {
        obj_pool_destroy(node->objs, pres->obj_reg);
    }
    
    obj_reg_destroy(pres->obj_reg);
    
    for (u32 i = 0; i < pres->num_plugins; i++) {
        os_lib_release(pres->plugins[i]);
    }
}

// TODO: make final version
void pres_draw(apres* pres, ap_app* app) {
    obj_pool_draw(pres->first_slide->objs, pres->obj_reg, app);
}
void pres_update(apres* pres, ap_app* app, f32 delta) {
    obj_pool_update(pres->first_slide->objs, pres->obj_reg, app, delta);
}
