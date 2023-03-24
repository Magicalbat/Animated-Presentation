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
} app_anim_repeat;

typedef struct {
    field_type type;
    void* obj_field;

    f64 time;
    cbezier bezier;
    app_anim_repeat repeat;

    u32 num_keys;
    void* keys;
    f64* times;
    b32* pauses;

    i32 cur_key;
    i32 next_key;
    f64 cur_time;
    i32 dir;
    u32 to_pause;
    b32 stopped;
} app_anim;

typedef struct {
    u32 max_anims;
    u32 num_anims;
    app_anim* anims;
} app_anim_pool;

app_anim_pool* app_animp_create(marena* arena, u32 max_anims);
app_anim* app_animp_next(app_anim_pool* pool);
void app_animp_update(app_anim_pool* apool, app_app* app, f32 delta);

void app_anim_finalize(marena* arena, app_anim* anim);

#ifdef __cplusplus
}
#endif

#endif // APP_ANIM_H
