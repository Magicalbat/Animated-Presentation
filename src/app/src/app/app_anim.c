#include "app/app_anim.h"
#include "app/app_app.h"

anim_pool* anim_pool_create(marena* arena, u32 max_anims) {
    anim_pool* apool = CREATE_STRUCT(arena, anim_pool);
    
    *apool = (anim_pool){
        .max_anims = max_anims,
        .anims = CREATE_ZERO_ARRAY(arena, anim, max_anims)
    };

    return apool;
}
void anim_pool_finalize(marena* arena, anim_pool* apool, u32 index) {
    anim* cur_anim = &apool->anims[index];

    if (cur_anim->num_keys == 0)
        return;
    if (cur_anim->keys == NULL) {
        log_error("Cannot finalize animation without keys");
        return;
    }

    if (cur_anim->pauses == NULL) {
        cur_anim->pauses = CREATE_ZERO_ARRAY(arena, b32, cur_anim->num_keys);
    }
    if (cur_anim->times == NULL) {
        cur_anim->times = CREATE_ARRAY(arena, f64, cur_anim->num_keys);
        
        for (u32 i = 0; i < cur_anim->num_keys; i++) {
            cur_anim->times[i] = cur_anim->total_time * ((f64)(i) / (f64)(cur_anim->num_keys - 1));
        }
    }
}
void anim_pool_update(anim_pool* apool, ap_app* app, f32 delta) {
    anim* cur_anim = apool->anims;
    for (u32 i = 0; i < apool->num_anims; i++, cur_anim += 1) {
        // Check for anims that are finished
        if (cur_anim->repeat == ANIM_STOP &&  cur_anim->cur_time == cur_anim->times[cur_anim->num_keys - 1]) {
            continue;
        }

        // Check for paused anims
        if (cur_anim->paused) {
            if (GFX_IS_KEY_JUST_DOWN(app->win, GFX_KEY_SPACE)) {
                cur_anim->paused = false;
            }
            if (cur_anim->paused)
                continue;
        }

        cur_anim->cur_time += (f64)(delta);

        // update value
        switch (cur_anim->type) {
            case FIELD_F64: {
                f64* data = (f64*)cur_anim->obj_field;
                
                f64 t = 
                    (cur_anim->cur_time - cur_anim->times[cur_anim->cur_key]) / 
                    (cur_anim->times[cur_anim->next_key] - cur_anim->times[cur_anim->cur_key]);
                
                *data = LERP(
                    cur_anim->keys[cur_anim->cur_key].val.f64,
                    cur_anim->keys[cur_anim->next_key].val.f64,
                    t
                );
            } break;
            case FIELD_STR8: { } break;
            case FIELD_BOOL32: { } break;
            case FIELD_VEC2D: { } break;
            case FIELD_VEC3D: { } break;
            case FIELD_VEC4D: { } break;
            default: {
                log_errorf("Invalid field type %d for animation", cur_anim->type);
            } break;
        }

        // Check for key advance
        if (cur_anim->cur_time >= cur_anim->times[cur_anim->next_key]) {
            if (cur_anim->pauses[cur_anim->next_key]) {
                cur_anim->paused = true;
            }
            
            cur_anim->cur_time = cur_anim->times[cur_anim->next_key];
            cur_anim->cur_key++;
            cur_anim->next_key++;
                
            if (cur_anim->next_key >= cur_anim->num_keys) {
                switch (cur_anim->repeat) {
                    case ANIM_STOP: {
                        cur_anim->cur_time = cur_anim->times[cur_anim->num_keys - 1];
                    } break;
                    case ANIM_LOOP: {
                        cur_anim->cur_time = 0;
                        cur_anim->cur_key = 0;
                        cur_anim->next_key = 1;
                        cur_anim->paused = cur_anim->pauses[0];
                    } break;
                    default: break;
                }
            }
        }
    }
}