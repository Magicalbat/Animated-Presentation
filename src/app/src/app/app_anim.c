#include "app/app_anim.h"
#include "app/app_app.h"

app_anim_pool* app_animp_create(marena* arena, u32 max_anims) {
    app_anim_pool* apool = CREATE_STRUCT(arena, app_anim_pool);
    
    *apool = (app_anim_pool){
        .max_anims = max_anims,
        .anims = CREATE_ZERO_ARRAY(arena, app_anim, max_anims)
    };

    return apool;
}
app_anim* app_animp_next(app_anim_pool* pool) {
    if (pool->num_anims + 1 > pool->max_anims) {
        log_error("Cannot get next anim, out of anims");
        return NULL;
    }

    app_anim* out = &pool->anims[pool->num_anims];
    pool->num_anims++;

    return out;
}

static void anim_update_val(app_anim* anim) {
    f64 t = 
        (anim->cur_time - anim->times[anim->cur_key]) / 
        (anim->times[anim->next_key] - anim->times[anim->cur_key]);

    switch (anim->type) {
        case FIELD_F64: {
            f64* data = (f64*)anim->obj_field;
            
            *data = LERP(
                ((f64*)anim->keys)[anim->cur_key],
                ((f64*)anim->keys)[anim->next_key],
                t
            );
        } break;
        case FIELD_STR8: {
            string8* data = (string8*)anim->obj_field;
            
            string8* target = &((string8*)anim->keys)[anim->cur_key];

            if (t < 0.5) {
                t = 1.0 - (t * 2.0);
            } else {
                t = (t - 0.5) * 2.0;
                target += 1;
            }

            data->str = target->str;
            data->size = (u64)(LERP(
                0.0, (f64)target->size, t
            ));
        } break;
        case FIELD_BOOL32: {
            b32* data = (b32*)anim->obj_field;

            *data = ((b32*)anim->keys)[anim->cur_key];
        } break;
        case FIELD_VEC2D: {
            vec2d* data = (vec2d*)anim->obj_field;

            vec2d cur = ((vec2d*)anim->keys)[anim->cur_key];
            vec2d next = ((vec2d*)anim->keys)[anim->next_key];

            data->x = LERP(cur.x, next.x, t);
            data->y = LERP(cur.y, next.y, t);
        } break;
        case FIELD_VEC3D: {
            vec3d* data = (vec3d*)anim->obj_field;

            vec3d cur = ((vec3d*)anim->keys)[anim->cur_key];
            vec3d next = ((vec3d*)anim->keys)[anim->next_key];

            data->x = LERP(cur.x, next.x, t);
            data->y = LERP(cur.y, next.y, t);
            data->z = LERP(cur.z, next.z, t);
        } break;
        case FIELD_VEC4D: {
            vec4d* data = (vec4d*)anim->obj_field;

            vec4d cur = ((vec4d*)anim->keys)[anim->cur_key];
            vec4d next = ((vec4d*)anim->keys)[anim->next_key];

            data->x = LERP(cur.x, next.x, t);
            data->y = LERP(cur.y, next.y, t);
            data->z = LERP(cur.z, next.z, t);
            data->w = LERP(cur.w, next.w, t);
        } break;
        default: {
            log_errorf("Invalid field type %d for animation", anim->type);
        } break;
    }
}

void app_animp_update(app_anim_pool* apool, app_app* app, f32 delta) {
    app_anim* anim = apool->anims;
    for (u32 i = 0; i < apool->num_anims; i++, anim += 1) {
        if (anim->repeat == ANIM_STOP && anim->stopped) {
            continue;
        }

        // Check for paused anims
        if (anim->to_pause) {
            if (GFX_IS_KEY_JUST_DOWN(app->win, GFX_KEY_SPACE)) {
                anim->to_pause--;
                anim_update_val(anim);
            }
            if (anim->to_pause)
                continue;
        }

        anim->cur_time += (f64)(delta);

        // Check for key advance
        if (anim->cur_time >= anim->times[anim->next_key]) {
            anim->to_pause += anim->pauses[anim->next_key];
            
            anim->cur_time = anim->times[anim->next_key];
            anim->cur_key++;
            anim->next_key++;
                
            if (anim->next_key >= anim->num_keys) {
                switch (anim->repeat) {
                    case ANIM_STOP: {
                        anim->cur_time = anim->times[anim->num_keys - 1];
                        anim->stopped = true;
                    } break;
                    case ANIM_LOOP: {
                        anim->cur_time = 0;
                        anim->cur_key = 0;
                        anim->next_key = 1;
                        anim->to_pause += anim->pauses[0];
                    } break;
                    // TODO: bounce
                    default: break;
                }
            }
        }

        if (!anim->to_pause)
            anim_update_val(anim);
    }
}

void app_anim_finalize(marena* arena, app_anim* anim) {
    if (anim->num_keys == 0)
        return;
    if (anim->keys == NULL) {
        log_error("Cannot finalize animation without keys");
        return;
    }
    anim->next_key = 1;

    if (anim->pauses == NULL) {
        anim->pauses = CREATE_ZERO_ARRAY(arena, b32, anim->num_keys);
    }
    anim->to_pause += anim->pauses[0];
    
    if (anim->times == NULL) {
        anim->times = CREATE_ARRAY(arena, f64, anim->num_keys);
        
        for (u32 i = 0; i < anim->num_keys; i++) {
            anim->times[i] = anim->time * ((f64)(i) / (f64)(anim->num_keys - 1));
        }
    }

    anim_update_val(anim);
}
