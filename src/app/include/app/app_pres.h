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

    slide_node* cur_slide;
} apres;

#define PRES_MAX_DESCS 32

apres* pres_parse(marena* arena, ap_app* app, string8 file_path);
void pres_delete(apres* pres);

void pres_draw(apres* pres, ap_app* app);
void pres_update(apres* pres, f32 delta);

#ifdef __cplusplus
}
#endif

#endif // APP_PRES_H
