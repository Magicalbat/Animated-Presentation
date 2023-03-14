#ifndef BASE_OBJ_H
#define BASE_OBJ_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"

typedef enum {
    FIELD_NULL,

    FIELD_F64,
    FIELD_STR8,
    FIELD_BOOL32,
    FIELD_VEC2D,
    FIELD_VEC3D,
    FIELD_VEC4D,

    FIELD_F64_ARR,
    FIELD_STR8_ARR,
    FIELD_BOOL_ARR,
    FIELD_VEC2D_ARR,
    FIELD_VEC3D_ARR,
    FIELD_VEC4D_ARR,

    FIELD_COUNT
} field_type;

typedef struct field_val {
    field_type type;

    union {
        struct { int _unused; } null;

        f64 f64;
        string8 str8;
        b32 bool32;
        vec2d vec2d;
        vec3d vec3d;
        vec4d vec4d;

        struct { u64 size; f64* data; } f64_arr;
        struct { u64 size; string8* data; } str8_arr;
        struct { u64 size; b32* data; } bool32_arr;
        struct { u64 size; vec2d* data; } vec2d_arr;
        struct { u64 size; vec3d* data; } vec3d_arr;
        struct { u64 size; vec4d* data; } vec4d_arr;
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

AP_EXPORT obj_register* obj_reg_create(marena* arena, u32 max_descs);
AP_EXPORT void obj_reg_add_desc(obj_register* obj_reg, obj_desc* desc);
AP_EXPORT void obj_reg_destroy(obj_register* obj_reg);

#ifdef __cplusplus
}
#endif

#endif // BASE_OBJ_H
