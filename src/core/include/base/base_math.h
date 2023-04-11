#ifndef BASE_MATH_H
#define BASE_MATH_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "base_defs.h"

#include <math.h>

typedef struct { f32 x, y, w, h; } rect;

typedef struct {
    f32 x, y;
} vec2;
typedef struct {
    f32 x, y, z;
} vec3;
typedef struct {
    f32 x, y, z, w;
} vec4;

typedef struct {
    f64 x, y;
} vec2d;
typedef struct {
    f64 x, y, z;
} vec3d;
typedef struct {
     f64 x, y, z, w;
} vec4d;

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

AP_EXPORT u64 datetime_to_sec(datetime t);
AP_EXPORT i64 datetime_diff_sec(datetime a, datetime b);

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

AP_EXPORT vec2d vec2d_add(vec2d a, vec2d b);
AP_EXPORT vec2d vec2d_sub(vec2d a, vec2d b);
AP_EXPORT vec2d vec2d_mul(vec2d a, f64   b);
AP_EXPORT vec2d vec2d_div(vec2d a, f64   b);
AP_EXPORT f64   vec2d_dot(vec2d a, vec2d b);
AP_EXPORT f64   vec2d_sql(vec2d          v);
AP_EXPORT f64   vec2d_len(vec2d          v);
AP_EXPORT vec2d vec2d_prp(vec2d          v);
AP_EXPORT vec2d vec2d_nrm(vec2d          v);

AP_EXPORT vec3d vec3d_add(vec3d a, vec3d b);
AP_EXPORT vec3d vec3d_sub(vec3d a, vec3d b);
AP_EXPORT vec3d vec3d_mul(vec3d a, f64   b);
AP_EXPORT vec3d vec3d_div(vec3d a, f64   b);
AP_EXPORT f64   vec3d_dot(vec3d a, vec3d b);
AP_EXPORT f64   vec3d_sql(vec3d          v);
AP_EXPORT f64   vec3d_len(vec3d          v);
AP_EXPORT vec3d vec3d_crs(vec3d a, vec3d b);
AP_EXPORT vec3d vec3d_nrm(vec3d v);

AP_EXPORT vec4d vec4d_add(vec4d a, vec4d b);
AP_EXPORT vec4d vec4d_sub(vec4d a, vec4d b);
AP_EXPORT vec4d vec4d_mul(vec4d a, f64   b);
AP_EXPORT vec4d vec4d_div(vec4d a, f64   b);
AP_EXPORT f64   vec4d_dot(vec4d a, vec4d b);
AP_EXPORT f64   vec4d_sql(vec4d          v);
AP_EXPORT f64   vec4d_len(vec4d          v);
AP_EXPORT vec4d vec4d_nrm(vec4d          v);

AP_EXPORT f32  qbezier_calc_x(qbezier* b, f32 t);
AP_EXPORT f32  qbezier_calc_y(qbezier* b, f32 t);
AP_EXPORT vec2 qbezier_calc(qbezier* b, f32 t);

// TODO: optimize bezier curves with pre-calculation?
// https://searchfox.org/mozilla-central/source/servo/components/style/bezier.rs

AP_EXPORT f32  cbezier_calc_x(cbezier* b, f32 t);
AP_EXPORT f32  cbezier_calc_y(cbezier* b, f32 t);
AP_EXPORT vec2 cbezier_calc(cbezier* b, f32 t);

AP_EXPORT f32  cbezier_calcd_x(cbezier* b, f32 t);
AP_EXPORT f32  cbezier_calcd_y(cbezier* b, f32 t);
AP_EXPORT vec2 cbezier_calcd(cbezier* b, f32 t);

AP_EXPORT f32 cbezier_solve(cbezier* b, f32 x);

#ifdef __cplusplus
}
#endif

#endif // BASE_MATH_H
