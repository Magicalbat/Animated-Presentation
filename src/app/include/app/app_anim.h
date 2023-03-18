#ifndef APP_ANIM_H
#define APP_ANIM_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"
#include "app/app_obj_pool.h"

typedef enum {
    ANIM_STOP,
    ANIM_LOOP,
    ANIM_BOUNCE
} anim_repeat;

typedef struct {
    field_type type;
    void* obj_field;

    f64 total_time;
    anim_repeat repeat;

    u32 num_keys;
    field_val* keys;
    f64* times;
    b32* pauses;

    u32 cur_key;
    u32 next_key;
    f64 cur_time;
    b32 paused;
} anim;

typedef struct {
    u32 max_anims;
    u32 num_anims;
    anim* anims;
} anim_pool;

anim_pool* anim_pool_create(marena* arena, u32 max_anims);
void anim_pool_finalize(marena* arena, anim_pool* apool, u32 index);
void anim_pool_update(anim_pool* apool, ap_app* app, f32 delta);

#ifdef __cplusplus
}
#endif

#endif // APP_ANIM_H
