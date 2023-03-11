#ifndef BASE_MATH_H
#define BASE_MATH_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base_defs.h"

#include <math.h>

typedef struct { f32 x, y, w, h; } rect;

typedef struct { f32 x, y; } vec2;
typedef struct { f32 x, y, z; } vec3;
typedef struct { f32 x, y, z, w; } vec4;

typedef union {
    struct {
        vec2 p0;
        vec2 p1;
        vec2 p2;
    };
    vec2 p[3];
} qbezier;

typedef union {
    struct {
        vec2 p0;
        vec2 p1;
        vec2 p2;
        vec2 p3;
    };
    vec2 p[4];
} cbezier;

AP_EXPORT rect rect_pack(rect* rects, u32 num_rects);

AP_EXPORT vec3 rgb_to_hsv(vec3 rgb);
AP_EXPORT vec3 hsv_to_rgb(vec3 hsv);

AP_EXPORT vec2 vec2_add(vec2 a, vec2 b);
AP_EXPORT vec2 vec2_sub(vec2 a, vec2 b);
AP_EXPORT vec2 vec2_mul(vec2 a, f32  b);
AP_EXPORT vec2 vec2_div(vec2 a, f32  b);
AP_EXPORT f32  vec2_dot(vec2 a, vec2 b);
AP_EXPORT f32  vec2_sql(vec2         v);
AP_EXPORT f32  vec2_len(vec2         v);
AP_EXPORT vec2 vec2_prp(vec2         v);
AP_EXPORT vec2 vec2_nrm(vec2         v);

AP_EXPORT vec3 vec3_add(vec3 a, vec3 b);
AP_EXPORT vec3 vec3_sub(vec3 a, vec3 b);
AP_EXPORT vec3 vec3_mul(vec3 a, f32  b);
AP_EXPORT vec3 vec3_div(vec3 a, f32  b);
AP_EXPORT f32  vec3_dot(vec3 a, vec3 b);
AP_EXPORT f32  vec3_sql(vec3         v);
AP_EXPORT f32  vec3_len(vec3         v);
AP_EXPORT vec3 vec3_crs(vec3 a, vec3 b);
AP_EXPORT vec3 vec3_nrm(vec3 v);

AP_EXPORT vec4 vec4_add(vec4 a, vec4 b);
AP_EXPORT vec4 vec4_sub(vec4 a, vec4 b);
AP_EXPORT vec4 vec4_mul(vec4 a, f32  b);
AP_EXPORT vec4 vec4_div(vec4 a, f32  b);
AP_EXPORT f32  vec4_dot(vec4 a, vec4 b);
AP_EXPORT f32  vec4_sql(vec4         v);
AP_EXPORT f32  vec4_len(vec4         v);
AP_EXPORT vec4 vec4_nrm(vec4         v);

AP_EXPORT f32  qbezier_calc_x(qbezier* b, f32 t);
AP_EXPORT f32  qbezier_calc_y(qbezier* b, f32 t);
AP_EXPORT vec2 qbezier_calc(qbezier* b, f32 t);

AP_EXPORT f32  cbezier_calc_x(cbezier* b, f32 t);
AP_EXPORT f32  cbezier_calc_y(cbezier* b, f32 t);
AP_EXPORT vec2 cbezier_calc(cbezier* b, f32 t);

AP_EXPORT f32  cbezier_calcd_x(cbezier* b, f32 t);
AP_EXPORT f32  cbezier_calcd_y(cbezier* b, f32 t);
AP_EXPORT vec2 cbezier_calcd(cbezier* b, f32 t);

#ifdef __cplusplus
}
#endif

#endif // BASE_MATH_H