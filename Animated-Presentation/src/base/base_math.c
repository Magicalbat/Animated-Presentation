#include "base_math.h"

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
