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
} obj_pool;

typedef struct {
    u32 desc_index;
    void* obj;
} obj_ref;

obj_pool* obj_pool_create(marena* arena, obj_register* obj_reg, u32 max_objs);
obj_ref obj_pool_add(obj_pool* pool, obj_register* obj_reg, string8 name, marena* arena, ap_app* app);
void obj_pool_update(obj_pool* pool, obj_register* obj_reg, ap_app* app, f32 delta);
void obj_pool_draw(obj_pool* pool, obj_register* obj_reg, ap_app* app);
void obj_pool_destroy(obj_pool* pool, obj_register* obj_reg);

void obj_ref_set(obj_ref ref, obj_register* obj_reg, string8 prop, void* data);
void obj_ref_init(obj_ref ref, obj_register* obj_reg, marena* arena, ap_app* app);

#ifdef __cplusplus
}
#endif

#endif // APP_OBJ_POOL_H
