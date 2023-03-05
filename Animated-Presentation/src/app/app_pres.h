#ifndef APP_PRES_H
#define APP_PRES_H

#include "base/base.h"
#include "app/app_obj.h"
#include "app/app_anim.h"

typedef struct slide_node {
    struct slide_node* next;
    struct slide_node* prev;

    obj_pool objs;
    anim_pool anims;
} slide_node;

typedef struct {
    obj_register* obj_reg;

    slide_node* first;
    slide_node* last;
    u32 num_slides;

    u32 cur_slide;
} pres;


void slide_draw(slide_node* slide, ap_app* app);
void slide_update(slide_node* slide, f32 delta);

#endif // APP_PRES_H
