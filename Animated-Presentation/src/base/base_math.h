#ifndef BASE_MATH_H
#define BASE_MATH_H

#include "base_def.h"

#include <math.h>

typedef struct { f32 x, y;       } vec2_t;
typedef struct { f32 x, y, z;    } vec3_t;
typedef struct { f32 x, y, z, w; } vec4_t;

inline vec2_t vec2_add( vec2_t a, vec2_t b ) { return (vec2_t) { a.x + b.x, a.y + b.y }; }
inline vec2_t vec2_sub( vec2_t a, vec2_t b ) { return (vec2_t) { a.x - b.x, a.y - b.y }; }
inline vec2_t vec2_mul( vec2_t a, f32  b   ) { return (vec2_t) { a.x * b, a.y * b };     }
inline vec2_t vec2_div( vec2_t a, f32  b   ) { return (vec2_t) { a.x / b, a.y / b };     }
inline f32    vec2_dot( vec2_t a, vec2_t b ) { return a.x * b.x + a.y * b.y;             }
inline f32    vec2_sql( vec2_t           v ) { return v.x * v.x + v.y * v.y;             }
inline f32    vec2_len( vec2_t           v ) { return sqrtf(v.x * v.x + v.y * v.y);      }
inline vec2_t vec2_prp( vec2_t           v ) { return (vec2_t) { -v.y, v.x };            }
vec2_t vec2_nrm( vec2_t           v );

inline vec3_t vec3_add( vec3_t a, vec3_t b ) { return (vec3_t) { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline vec3_t vec3_sub( vec3_t a, vec3_t b ) { return (vec3_t) { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline vec3_t vec3_mul( vec3_t a, f32  b   ) { return (vec3_t) { a.x * b, a.y * b, a.z * b };       }
inline vec3_t vec3_div( vec3_t a, f32  b   ) { return (vec3_t) { a.x / b, a.y / b, a.z / b };       }
inline f32    vec3_dot( vec3_t a, vec3_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z;            }
inline f32    vec3_sql( vec3_t           v ) { return v.x * v.x + v.y * v.y + v.z + v.z;            }
inline f32    vec3_len( vec3_t           v ) { return sqrtf(v.x * v.x + v.y * v.y + v.z + v.z);     }
inline vec3_t vec3_crs( vec3_t a, vec3_t b ) {
	return (vec3_t) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
vec3_t vec3_nrm(vec3_t v);

inline vec4_t vec4_add( vec4_t a, vec4_t b ) { return (vec4_t) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
inline vec4_t vec4_sub( vec4_t a, vec4_t b ) { return (vec4_t) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
inline vec4_t vec4_mul( vec4_t a, f32  b   ) { return (vec4_t) { a.x * b, a.y * b, a.z * b, a.w * b };         }
inline vec4_t vec4_div( vec4_t a, f32  b   ) { return (vec4_t) { a.x / b, a.y / b, a.z / b, a.w / b };         }
inline f32    vec4_dot( vec4_t a, vec4_t b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;           }
inline f32    vec4_sql( vec4_t           v ) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;           }
inline f32    vec4_len( vec4_t           v ) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);    }
vec4_t vec4_nrm( vec4_t           v );

#endif // BASE_MATH_H