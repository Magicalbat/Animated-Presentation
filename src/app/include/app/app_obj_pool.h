#ifndef APP_OBJ_POOL_H
#define APP_OBJ_POOL_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"

typedef struct {
    u32 max_objs;
    u32* num_objs;
    void** objs;
} app_obj_pool;

typedef struct {
    u32 desc_index;
    void* obj;
} app_obj_ref;

app_obj_pool* app_objp_create(marena* arena, obj_register* obj_reg, u32 max_objs);
app_obj_ref app_objp_add(app_obj_pool* pool, obj_register* obj_reg, string8 name, marena* arena, app_app* app);
void app_objp_file(marena* arena, app_obj_pool* pool, obj_register* obj_reg, app_app* app);
void app_objp_update(app_obj_pool* pool, obj_register* obj_reg, app_app* app, f32 delta);
void app_objp_draw(app_obj_pool* pool, obj_register* obj_reg, app_app* app);
void app_objp_destroy(app_obj_pool* pool, obj_register* obj_reg);

void app_obj_ref_set(app_obj_ref ref, obj_register* obj_reg, string8 prop, void* data);
void app_obj_ref_init(app_obj_ref ref, obj_register* obj_reg, marena* arena, app_app* app);

#ifdef __cplusplus
}
#endif

#endif // APP_OBJ_POOL_H
