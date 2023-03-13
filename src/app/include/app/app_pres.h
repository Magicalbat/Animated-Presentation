#ifndef APP_PRES_H
#define APP_PRES_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "os/os.h"
#include "app_app.h"
#include "app_anim.h"
#include "app_obj_pool.h"

typedef struct slide_node {
    struct slide_node* next;
    struct slide_node* prev;

    obj_pool* objs;
    anim_pool* anims;
} slide_node;

typedef struct apres {
    u32 num_plugins;
    os_library* plugins;

    obj_register* obj_reg;

    slide_node* first_slide;
    slide_node* last_slide;
    u32 num_slides;
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
