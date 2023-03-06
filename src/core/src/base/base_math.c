#include "base/base_math.h"
#include "base/base_log.h"

// I could try different algorithms if I wanted to
// With random rectangles it averages about 80% packing density
rect rect_pack(rect* rects, u32 num_rects) {
    marena_temp scratch = marena_scratch_get(NULL, 0);

    b32* rect_used = CREATE_ZERO_ARRAY(scratch.arena, b32, num_rects);
    rect* openings = CREATE_ZERO_ARRAY(scratch.arena, rect, num_rects);

    rect* tallest = &(rect){ 0 };
    u32 tallest_index = 0;
    for (u32 j = 0; j < num_rects; j++) {
        if (rects[j].h > tallest->h) {
            tallest = &rects[j];
            tallest_index = j;
        }
    }
    rect_used[tallest_index] = true;

    f32 height = rects[tallest_index].h;
    f32 width = rects[tallest_index].w;

    for (u32 i = 1; i < num_rects; i++) {
        rect* widest = &(rect){ 0 };
        u32 widest_index = 0;
        for (u32 j = 0; j < num_rects; j++) {
            if (rects[j].w > widest->w && !rect_used[j]) {
                widest = &rects[j];
                widest_index = j;
            }
        }
        rect_used[widest_index] = true;

        b32 opening_found = false;
        for (u32 j = i; j > 0; j--) {
            if (widest->h <= openings[j].h && widest->w <= openings[j].w) {
                widest->x = openings[j].x;
                widest->y = openings[j].y;

                openings[j].y += widest->h;
                openings[j].h -= widest->h;

                opening_found = true;
            }
        }

        if (opening_found)  continue;

        widest->x = width;
        widest->y = 0;
        width += widest->w;

        openings[i] = (rect){
            .x = widest->x,
            .y = widest->h + 1,
            .w = widest->w,
            .h = height - widest->h
        };
    }

    marena_scratch_release(scratch);
    return (rect){ 
        .w = width,
        .h = height
    };
}

// https://github.com/python/cpython/blob/3.11/Lib/colorsys.py
vec3 rgb_to_hsv(vec3 rgb) {
    f32 cmax = MAX(rgb.x, MAX(rgb.y, rgb.z));
    f32 cmin = MIN(rgb.x, MIN(rgb.y, rgb.z));
    f32 diff = cmax - cmin;

    vec3 hsv = { .z = cmax };

    if (cmax == cmin) {
        return hsv;
    }

    hsv.y = diff / cmax;
    if (cmax == rgb.x) {
        hsv.x = (rgb.y - rgb.z) / diff;
    } else if (cmax == rgb.y) {
        hsv.x = 2.0f + (rgb.z - rgb.x) / diff;
    } else {
        hsv.x = 4.0f + (rgb.x - rgb.y) / diff;
    }
    
    hsv.x = fmodf(hsv.x / 6.0f, 1.0f);

    return hsv;
}
vec3 hsv_to_rgb(vec3 hsv) {
    if (hsv.y == 0.0f)
        return (vec3){ hsv.z, hsv.z, hsv.z };

    f32 v = hsv.z;
    
    u32 i = (int)(hsv.x * 6.0f);
    f32 f = (hsv.x * 6.0f) - i;
    f32 p = v * (1.0f - hsv.y);
    f32 q = v * (1.0f - hsv.y * f);
    f32 t = v * (1.0f - hsv.y * (1.0f - f));
    
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

vec2 vec2_add(vec2 a, vec2 b) { return (vec2) { a.x + b.x, a.y + b.y }; }
vec2 vec2_sub(vec2 a, vec2 b) { return (vec2) { a.x - b.x, a.y - b.y }; }
vec2 vec2_mul(vec2 a, f32  b) { return (vec2) { a.x * b, a.y * b };     }
vec2 vec2_div(vec2 a, f32  b) { return (vec2) { a.x / b, a.y / b };     }
f32  vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y;           }
f32  vec2_sql(vec2         v) { return v.x * v.x + v.y * v.y;           }
f32  vec2_len(vec2         v) { return sqrtf(v.x * v.x + v.y * v.y);    }
vec2 vec2_prp(vec2         v) { return (vec2) { -v.y, v.x };            }
vec2 vec2_nrm(vec2 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y);
    ASSERT(l != 0.0f, "Cannot normalize vec2 of length 0.");
    l = 1 / l;
    return (vec2) { v.x * l, v.y * l };
}

vec3 vec3_add(vec3 a, vec3 b) { return (vec3) { a.x + b.x, a.y + b.y, a.z + b.z }; }
vec3 vec3_sub(vec3 a, vec3 b) { return (vec3) { a.x - b.x, a.y - b.y, a.z - b.z }; }
vec3 vec3_mul(vec3 a, f32  b) { return (vec3) { a.x * b, a.y * b, a.z * b };       }
vec3 vec3_div(vec3 a, f32  b) { return (vec3) { a.x / b, a.y / b, a.z / b };       }
f32  vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z;          }
f32  vec3_sql(vec3         v) { return v.x * v.x + v.y * v.y + v.z + v.z;          }
f32  vec3_len(vec3         v) { return sqrtf(v.x * v.x + v.y * v.y + v.z + v.z);   }
vec3 vec3_crs(vec3 a, vec3 b) {
    return (vec3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
vec3 vec3_nrm(vec3 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    ASSERT(l != 0, "Cannot normalize vec3 of length 0.");
    l = 1 / l;
    return (vec3) { v.x * l, v.y * l, v.z * l };
}

vec4 vec4_add(vec4 a, vec4 b) { return (vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
vec4 vec4_sub(vec4 a, vec4 b) { return (vec4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
vec4 vec4_mul(vec4 a, f32  b) { return (vec4) { a.x * b, a.y * b, a.z * b, a.w * b };         }
vec4 vec4_div(vec4 a, f32  b) { return (vec4) { a.x / b, a.y / b, a.z / b, a.w / b };         }
f32  vec4_dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;         }
f32  vec4_sql(vec4         v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;         }
f32  vec4_len(vec4         v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);  }
vec4 vec4_nrm(vec4 v) {
    f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    ASSERT(l != 0, "Cannot normalize vec4 of length 0.");
    l = 1 / l;
    return (vec4) { v.x * l, v.y * l, v.z * l, v.w * l };
}

f32 qbezier_calc_x(qbezier* b, f32 t) {
    return b->p1.x + (1 - t) * (1 - t) * (b->p0.x - b->p1.x) + t * t * (b->p2.x - b->p1.x);
}
f32 qbezier_calc_y(qbezier* b, f32 t) {
    return b->p1.y + (1 - t) * (1 - t) * (b->p0.y - b->p1.y) + t * t * (b->p2.y - b->p1.y);
}
vec2 qbezier_calc(qbezier* b, f32 t) {
    return (vec2){
        .x = b->p1.x + (1 - t) * (1 - t) * (b->p0.x - b->p1.x) + t * t * (b->p2.x - b->p1.x),
        .y = b->p1.y + (1 - t) * (1 - t) * (b->p0.y - b->p1.y) + t * t * (b->p2.y - b->p1.y)
    };
}

f32 cbezier_calc_x(cbezier* b, f32 t) {
    return (1 - t) * (1 - t) * (1 - t) * b->p0.x +
           3 * (1 - t) * (1 - t) * t * b->p1.x + 
           3 * (1 - t) * t * t * b->p2.x + 
           t * t * t * b->p3.x;
}
f32 cbezier_calc_y(cbezier* b, f32 t) {
    return (1 - t) * (1 - t) * (1 - t) * b->p0.y +
           3 * (1 - t) * (1 - t) * t * b->p1.y + 
           3 * (1 - t) * t * t * b->p2.y + 
           t * t * t * b->p3.y;
}
vec2 cbezier_calc(cbezier* b, f32 t) {
    return (vec2){
        .x = (1 - t) * (1 - t) * (1 - t) * b->p0.x +
             3 * (1 - t) * (1 - t) * t * b->p1.x + 
             3 * (1 - t) * t * t * b->p2.x + 
             t * t * t * b->p3.x,
        .y = (1 - t) * (1 - t) * (1 - t) * b->p0.y +
             3 * (1 - t) * (1 - t) * t * b->p1.y + 
             3 * (1 - t) * t * t * b->p2.y + 
             t * t * t * b->p3.y
    };
}
f32 cbezier_calcd_x(cbezier* b, f32 t) {
    return 3 * (1 - t) * (1 - t) * (b->p1.x - b->p0.x) +
           6 * (1 - t) * t * (b->p2.x - b->p1.x) +
           3 * t * t * (b->p3.x - b->p2.x);
}
f32 cbezier_calcd_y(cbezier* b, f32 t) {
    return 3 * (1 - t) * (1 - t) * (b->p1.y - b->p0.y) +
           6 * (1 - t) * t * (b->p2.y - b->p1.y) +
           3 * t * t * (b->p3.y - b->p2.y);
}
vec2 cbezier_calcd(cbezier* b, f32 t) {
    return (vec2){
        .x = 3 * (1 - t) * (1 - t) * (b->p1.x - b->p0.x) +
             6 * (1 - t) * t * (b->p2.x - b->p1.x) +
             3 * t * t * (b->p3.x - b->p2.x),
        .y = 3 * (1 - t) * (1 - t) * (b->p1.y - b->p0.y) +
             6 * (1 - t) * t * (b->p2.y - b->p1.y) +
             3 * t * t * (b->p3.y - b->p2.y)
    };
}
