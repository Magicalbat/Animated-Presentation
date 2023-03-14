#include "app/app_obj_pool.h"

typedef struct { u64 size; f64* data; } null_arr;

static const u32 field_sizes[FIELD_COUNT] = {
    0,

    sizeof(f64),
    sizeof(string8),
    sizeof(b32),
    sizeof(vec2),
    sizeof(vec3),
    sizeof(vec4),

    sizeof(null_arr),
    sizeof(null_arr),
    sizeof(null_arr),
    sizeof(null_arr),
    sizeof(null_arr),
    sizeof(null_arr),
};

obj_pool* obj_pool_create(marena* arena, obj_register* obj_reg, u32 max_objs) {
    obj_pool* pool = CREATE_STRUCT(arena, obj_pool);

    *pool = (obj_pool){
        .max_objs = max_objs,
        .num_objs = CREATE_ZERO_ARRAY(arena, u32, obj_reg->num_descs),
        .objs = CREATE_ZERO_ARRAY(arena, void*, obj_reg->num_descs)
    };

    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        pool->objs[i] = (void*)marena_push_zero(arena, obj_reg->descs[i].obj_size);
    }

    return pool;
}
obj_ref obj_pool_add(obj_pool* pool, obj_register* obj_reg, string8 name) {
    i64 desc_index = -1;
    for (u32 i = 0; i < obj_reg->num_descs; i++) {
        if (str8_equals(name, obj_reg->descs[i].name)){ 
            desc_index = i;
            break;
        }
    }
    if (desc_index == -1) {
        log_errorf("Failed to locate object \"%.*s\" in obj register", (int)name.size, (char*)name.str);
        return (obj_ref){ 0 };
    }


    u32 index = pool->num_objs[desc_index];
    void* obj = (void*)((u8*)pool->objs[desc_index] + obj_reg->descs[desc_index].obj_size * index);

    obj_ref out = {
        .desc_index = (u32)desc_index,
        .obj = obj
    };

    pool->num_objs[desc_index]++;

    if (obj_reg->descs[desc_index].init_func != NULL) {
        obj_reg->descs[desc_index].init_func(obj);
    }

    return out;
}

#define CALL_OBJ_FUNCS(func, ...) \
    for (u32 i = 0; i < obj_reg->num_descs; i++) { \
        if (obj_reg->descs[i].func == NULL) \
            continue; \
        u32 obj_size = obj_reg->descs[i].obj_size; \
        void* obj = pool->objs[i]; \
        for (u32 j = 0; j < pool->num_objs[i]; j++) { \
            obj_reg->descs[i].func(__VA_ARGS__); \
            obj = (void*)((u8*)obj + obj_size); \
        } \
    }

void obj_pool_draw(obj_pool* pool, obj_register* obj_reg, ap_app* app) {
    CALL_OBJ_FUNCS(draw_func, app, obj);
}
void obj_pool_update(obj_pool* pool, obj_register* obj_reg, f32 delta) {
    CALL_OBJ_FUNCS(update_func, delta, obj);
}
void obj_pool_destroy(obj_pool* pool, obj_register* obj_reg) {
    CALL_OBJ_FUNCS(destroy_func, obj);
}

void obj_ref_set(obj_ref ref, obj_register* obj_reg, string8 prop, void* data) {
    obj_desc* desc = &obj_reg->descs[ref.desc_index];

    i64 field_index = -1;
    for (u32 i = 0; i < DESC_MAX_FIELDS; i++) {
        if (desc->field_types[i] == FIELD_NULL) break;

        if (str8_equals(prop, desc->field_names[i])) {
            field_index = i;
            break;
        }
    }

    if (field_index == -1) {
        log_errorf("Failed to find field \"%.*s\"", (int)prop.size, (char*)prop.str);
        return;
    }

    u32 field_size = field_sizes[desc->field_types[field_index]];
    void* field_ptr = (void*)((u8*)ref.obj + desc->field_offsets[field_index]);

    memcpy(field_ptr, data, field_size);
}
