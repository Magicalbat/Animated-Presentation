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

#define PRES_MAX_OBJS 64
#define PRES_MAX_ANIMS (PRES_MAX_OBJS * 2)

typedef struct app_slide_node {
    struct app_slide_node* next;
    struct app_slide_node* prev;

    app_obj_pool* objs;
    app_anim_pool* anims;
} app_slide_node;

typedef struct app_pres {
    u32 num_plugins;
    os_library* plugins;

    obj_register* obj_reg;

    app_slide_node* first_slide;
    app_slide_node* last_slide;
    u32 num_slides;

    u32 slide_index;
    app_slide_node* cur_slide;

    app_slide_node* global_slide;
} app_pres;

#define PRES_MAX_DESCS 32

app_pres* app_pres_parse(marena* arena, app_app* app, string8 file_path);
void app_pres_destroy(app_pres* pres);

void app_pres_next_slide(app_pres* pres);
void app_pres_prev_slide(app_pres* pres);

void app_pres_draw(app_pres* pres, app_app* app);
void app_pres_update(app_pres* pres, app_app* app, f32 delta);

#ifdef __cplusplus
}
#endif

#endif // APP_PRES_H
