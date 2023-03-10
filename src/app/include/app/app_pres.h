#ifndef APP_PRES_H
#define APP_PRES_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "os/os.h"
#include "app/app_obj_pool.h"
#include "app/app_anim.h"

typedef struct slide_node {
    struct slide_node* next;
    struct slide_node* prev;

    obj_pool objs;
    anim_pool anims;
} slide_node;

typedef struct {
    u32 num_plugins;
    os_library* plugins;

    obj_register* obj_reg;

    slide_node* first;
    slide_node* last;
    u32 num_slides;

    u32 cur_slide;
} apres;

apres* pres_parse(marena* arena, string8 file_path);

void slide_draw(slide_node* slide, ap_app* app);
void slide_update(slide_node* slide, f32 delta);

#ifdef __cplusplus
}
#endif

#endif // APP_PRES_H
