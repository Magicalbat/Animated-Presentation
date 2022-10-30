#include "base_math.h"

vec2_t vec2_add( vec2_t a, vec2_t b ) { return (vec2_t) { a.x + b.x, a.y + b.y }; }
vec2_t vec2_sub( vec2_t a, vec2_t b ) { return (vec2_t) { a.x - b.x, a.y - b.y }; }
vec2_t vec2_mul( vec2_t a, float  b ) { return (vec2_t) { a.x * b, a.y * b };     }
vec2_t vec2_div( vec2_t a, float  b ) { return (vec2_t) { a.x / b, a.y / b };     }
float  vec2_dot( vec2_t a, vec2_t b )	{ return a.x * b.x + a.y * b.y;             }
float  vec2_sql( vec2_t           v ) { return v.x * v.x + v.y * v.y;             }
float  vec2_len( vec2_t           v ) { return sqrtf(v.x * v.x + v.y * v.y);      }
vec2_t vec2_prp( vec2_t           v ) { return (vec2_t) { -v.y, v.x };            }
vec2_t vec2_nrm( vec2_t           v ) {
	float l = sqrtf(v.x * v.x + v.y * v.y);
	ASSERT(l != 0.0f && "Cannot normalize vec2_t of length 0.");
	l = 1 / l;
	return (vec2_t) { v.x * l, v.y * l };
}

vec3_t vec3_add( vec3_t a, vec3_t b ) { return (vec3_t) { a.x + b.x, a.y + b.y, a.z + b.z }; }
vec3_t vec3_sub( vec3_t a, vec3_t b ) { return (vec3_t) { a.x - b.x, a.y - b.y, a.z - b.z }; }
vec3_t vec3_mul( vec3_t a, float  b ) { return (vec3_t) { a.x * b, a.y * b, a.z * b };       }
vec3_t vec3_div( vec3_t a, float  b ) { return (vec3_t) { a.x / b, a.y / b, a.z / b };       }
float  vec3_dot( vec3_t a, vec3_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z;            }
float  vec3_sql( vec3_t           v ) { return v.x * v.x + v.y * v.y + v.z + v.z;            }
float  vec3_len( vec3_t           v ) { return sqrtf(v.x * v.x + v.y * v.y + v.z + v.z);     }
vec3_t vec3_crs( vec3_t a, vec3_t b ) {
	return (vec3_t) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
vec3_t vec3_nrm(vec3_t v) {
	float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	ASSERT(l != 0 && "Cannot normalize vec3_t of length 0.");
	l = 1 / l;
	return (vec3_t) { v.x * l, v.y * l, v.z * l };
}

vec4_t vec4_add( vec4_t a, vec4_t b ) { return (vec4_t) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
vec4_t vec4_sub( vec4_t a, vec4_t b ) { return (vec4_t) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
vec4_t vec4_mul( vec4_t a, float  b ) { return (vec4_t) { a.x * b, a.y * b, a.z * b, a.w * b };         }
vec4_t vec4_div( vec4_t a, float  b ) { return (vec4_t) { a.x / b, a.y / b, a.z / b, a.w / b };         }
float  vec4_dot( vec4_t a, vec4_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;           }
float  vec4_sql( vec4_t           v ) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;           }
float  vec4_len( vec4_t           v ) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);    }
vec4_t vec4_nrm( vec4_t           v ) {
	float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
	ASSERT(l != 0 && "Cannot normalize vec4_t of length 0.");
	l = 1 / l;
	return (vec4_t) { v.x * l, v.y * l, v.z * l, v.w * l };
}
