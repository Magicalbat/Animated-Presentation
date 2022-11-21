#include "base_math.h"

vec2_t vec2_nrm( vec2_t           v ) {
	f32 l = sqrtf(v.x * v.x + v.y * v.y);
	ASSERT(l != 0.0f, "Cannot normalize vec2_t of length 0.");
	l = 1 / l;
	return (vec2_t) { v.x * l, v.y * l };
}

vec3_t vec3_nrm(vec3_t v) {
	f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	ASSERT(l != 0, "Cannot normalize vec3_t of length 0.");
	l = 1 / l;
	return (vec3_t) { v.x * l, v.y * l, v.z * l };
}

vec4_t vec4_nrm( vec4_t           v ) {
	f32 l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	ASSERT(l != 0, "Cannot normalize vec4_t of length 0.");
	l = 1 / l;
	return (vec4_t) { v.x * l, v.y * l, v.z * l, v.w * l };
}
