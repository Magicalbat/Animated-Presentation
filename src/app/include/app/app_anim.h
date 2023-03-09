#ifndef APP_ANIM_H
#define APP_ANIM_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "app/app_obj_pool.h"

typedef struct {
    field_type type;
    void* obj_field;

    u32 num_keys;
    field_val* keys;
    f32* times;

    f32 cur_time;
} anim;

typedef struct {
    anim* anims;
    u32 num_anims;
} anim_pool;

#ifdef __cplusplus
}
#endif

#endif // APP_ANIM_H
