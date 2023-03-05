#ifndef APP_OBJ_H
#define APP_OBJ_H

#include "base/base.h"

typedef enum {
    FIELD_NULL,
    FIELD_NUM,
    FIELD_STR,
    FIELD_BOOL,
    FIELD_VEC2,
    FIELD_VEC3,
    FIELD_VEC4,
    FIELD_ARR,

    FIELD_COUNT
} field_type;

typedef struct field_val {
    field_type type;

    union {
        struct { int _unused; } null;
        f64 num;
        string8 str;
        b32 boolean;
        vec2 vec2;
        vec3 vec3;
        vec4 vec4;
        struct {
            struct field_val* data;
            u64 size;
        } arr;
    } val;
} field_val;

typedef struct ap_app ap_app;

typedef void (desc_init_func)(void* custom_data);
typedef void (desc_destroy_func)(void* custom_data);

typedef void (obj_init_func)(void* obj);
typedef void (obj_destroy_func)(void* obj);
typedef void (obj_draw_func)(ap_app* app, void* obj);
typedef void (obj_update_func)(f32 delta, void* obj);

#define DESC_MAX_FIELDS 32
typedef struct {
    string8 name;
    u32 obj_size;

    void* custom_data;
    desc_init_func* desc_init_func;
    desc_destroy_func* desc_destroy_func;

    obj_init_func* init_func;
    obj_destroy_func* destroy_func;
    obj_draw_func* draw_func;
    obj_update_func* update_func;

    string8 field_names[DESC_MAX_FIELDS];
    field_type field_types[DESC_MAX_FIELDS];
    u32 field_offsets[DESC_MAX_FIELDS];
} obj_desc;

typedef struct {
    obj_desc* descs;
    u32 max_descs;
    u32 num_descs;
} obj_register;

typedef struct {
    u32 max_objs;
    u32* num_objs;
    void** objs;
} obj_pool;

typedef struct {
    u32 desc_index;
    void* obj;
} obj_ref;

obj_register* obj_reg_create(marena* arena, u32 max_descs);
void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc);
void obj_reg_destroy(obj_register* obj_reg);

obj_pool* obj_pool_create(marena* arena, obj_register* obj_reg, u32 max_objs);
obj_ref obj_pool_add(obj_pool* pool, obj_register* obj_reg, string8 name);
void obj_pool_update(obj_pool* pool, obj_register* obj_reg, f32 delta);
void obj_pool_draw(obj_pool* pool, obj_register* obj_reg, ap_app* app);
void obj_pool_destroy(obj_pool* pool, obj_register* obj_reg);

void obj_ref_set(obj_ref ref, obj_register* obj_reg, string8 prop, void* data);

#endif // APP_OBJ_H
