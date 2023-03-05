#ifndef APP_ANIM_H
#define APP_ANIM_H

#include "base/base.h"
#include "app/app_obj.h"

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


#endif // APP_ANIM_H
