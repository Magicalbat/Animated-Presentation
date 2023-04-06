#ifndef BASE_OBJ_H
#define BASE_OBJ_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base/base.h"

#define FIELD_XLIST \
    X(NULL) \
    X(F64) \
    X(STR8) \
    X(BOOL32) \
    X(VEC2D) \
    X(VEC3D) \
    X(VEC4D) \
    X(F64_ARR) \
    X(STR8_ARR) \
    X(BOOL32_ARR) \
    X(VEC2D_ARR) \
    X(VEC3D_ARR) \
    X(VEC4D_ARR)

typedef enum {
#define X(a) FIELD_##a,
    FIELD_XLIST
#undef X

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

typedef struct app_app app_app;

typedef void (desc_init_func)(marena* arena, app_app* app);
typedef void (desc_destroy_func)(void);

typedef void (obj_default_func)(marena* arena, app_app* app, void* obj);
typedef void (obj_init_func)(marena* arena, app_app* app, void* obj);
typedef void (obj_file_func)(marena* arena, app_app* app, void* obj);
typedef void (obj_destroy_func)(void* obj);
typedef void (obj_draw_func)(app_app* app, void* obj);
typedef void (obj_update_func)(app_app* app, void* obj, f32 delta);

typedef struct {
    string8 name;
    field_type type;
    u32 offset;
} field_desc;

#define OBJ_DESC_MAX_FIELDS 32
typedef struct {
    string8 name;
    u32 obj_size;

    desc_init_func* desc_init_func;
    desc_destroy_func* desc_destroy_func;

    obj_default_func* default_func;
    obj_init_func* init_func;
    obj_file_func* file_func;
    obj_destroy_func* destroy_func;
    obj_draw_func* draw_func;
    obj_update_func* update_func;

    field_desc fields[OBJ_DESC_MAX_FIELDS];
} obj_desc;

typedef struct {
    obj_desc* descs;
    u32 max_descs;
    u32 num_descs;
} obj_register;

AP_EXPORT obj_register* obj_reg_create(marena* arena, u32 max_descs);
AP_EXPORT void obj_reg_add_desc(marena* arena, app_app* app, obj_register* obj_reg, obj_desc* desc);
AP_EXPORT void obj_reg_destroy(obj_register* obj_reg);

#ifdef __cplusplus
}
#endif

#endif // BASE_OBJ_H
