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

void app_pres_next_slide(app_pres* pres) {
    if (pres->cur_slide->next != NULL) {
        pres->cur_slide = pres->cur_slide->next;
        pres->slide_index++;
    }
}
void app_pres_prev_slide(app_pres* pres) {
    if (pres->cur_slide->prev != NULL) {
        pres->cur_slide = pres->cur_slide->prev;
        pres->slide_index--;
    }
}

void app_pres_draw(app_pres* pres, app_app* app) {
    if (pres->cur_slide != NULL)
        app_objp_draw(pres->cur_slide->objs, pres->obj_reg, app);

    if (pres->global_slide != NULL)
        app_objp_draw(pres->global_slide->objs, pres->obj_reg, app);
}
void app_pres_update(app_pres* pres, app_app* app, f32 delta) {
    if (pres->cur_slide != NULL) {
        if (GFX_IS_KEY_JUST_DOWN(app->win, GFX_KEY_RIGHT)) {
            app_pres_next_slide(pres);
        }
        if (GFX_IS_KEY_JUST_DOWN(app->win, GFX_KEY_LEFT)) {
            app_pres_prev_slide(pres);
        }
        
        app_animp_update(pres->cur_slide->anims, app, delta);
        app_objp_update(pres->cur_slide->objs, pres->obj_reg, app, delta);
    }

    if (pres->global_slide != NULL) {
        app_animp_update(pres->global_slide->anims, app, delta);
        app_objp_update(pres->global_slide->objs, pres->obj_reg, app, delta);
    }
}
