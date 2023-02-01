#include "base_math.h"
#include "base_log.h"

// https://github.com/python/cpython/blob/3.11/Lib/colorsys.py
vec3 rgb_to_hsv(vec3 rgb) {
    f32 cmax = MAX(rgb.r, MAX(rgb.g, rgb.b));
    f32 cmin = MIN(rgb.r, MIN(rgb.g, rgb.b));
    f32 diff = cmax - cmin;

    vec3 hsv = { .v = cmax };

    if (cmax == cmin) {
        return hsv;
    }

    hsv.s = diff / cmax;
    if (cmax == rgb.r) {
        hsv.h = (rgb.g - rgb.b) / diff;
    } else if (cmax == rgb.g) {
        hsv.h = 2.0f + (rgb.b - rgb.r) / diff;
    } else {
        hsv.h = 4.0f + (rgb.r - rgb.g) / diff;
    }
    
    hsv.h = fmodf(hsv.h / 6.0f, 1.0f);

    return hsv;
}
vec3 hsv_to_rgb(vec3 hsv) {
    if (hsv.s == 0.0f)
        return (vec3){ hsv.v, hsv.v, hsv.v };

    f32 v = hsv.v;
    
    u32 i = (int)(hsv.h * 6.0f);
    f32 f = (hsv.h * 6.0f) - i;
    f32 p = v * (1.0f - hsv.s);
    f32 q = v * (1.0f - hsv.s * f);
    f32 t = v * (1.0f - hsv.s * (1.0f - f));
    
    i %= 6;

    switch (i) {
        case 0:    return (vec3){ v, t, p };
        case 1:    return (vec3){ q, v, p };
        case 2:    return (vec3){ p, v, t };
        case 3:    return (vec3){ p, q, v };
        case 4:    return (vec3){ t, p, v };
        case 5:    return (vec3){ v, p, q };
        default: log_error("Invalid HSV, cannot convert");
    }
    
    return (vec3){ 0 };
}

vec2 vec2_nrm(vec2 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y);
    ASSERT(l != 0.0f, "Cannot normalize vec2 of length 0.");
    l = 1 / l;
    return (vec2) { v.x * l, v.y * l };
}

vec3 vec3_nrm(vec3 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    ASSERT(l != 0, "Cannot normalize vec3 of length 0.");
    l = 1 / l;
    return (vec3) { v.x * l, v.y * l, v.z * l };
}

vec4 vec4_nrm(vec4 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    ASSERT(l != 0, "Cannot normalize vec4 of length 0.");
    l = 1 / l;
    return (vec4) { v.x * l, v.y * l, v.z * l, v.w * l };
}
