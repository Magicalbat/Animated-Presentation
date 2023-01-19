#ifndef BASE_MATH_H
#define BASE_MATH_H

#include "base_defs.h"

#include <math.h>

typedef struct { f32 x, y, w, h; } rect;

typedef struct { f32 x, y;       } vec2;
typedef struct { f32 x, y, z;    } vec3;
typedef struct { f32 x, y, z, w; } vec4;

inline vec2 vec2_add(vec2 a, vec2 b) { return (vec2) { a.x + b.x, a.y + b.y }; }
inline vec2 vec2_sub(vec2 a, vec2 b) { return (vec2) { a.x - b.x, a.y - b.y }; }
inline vec2 vec2_mul(vec2 a, f32  b) { return (vec2) { a.x * b, a.y * b };     }
inline vec2 vec2_div(vec2 a, f32  b) { return (vec2) { a.x / b, a.y / b };     }
inline f32  vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y;           }
inline f32  vec2_sql(vec2         v) { return v.x * v.x + v.y * v.y;           }
inline f32  vec2_len(vec2         v) { return sqrtf(v.x * v.x + v.y * v.y);    }
inline vec2 vec2_prp(vec2         v) { return (vec2) { -v.y, v.x };            }
vec2        vec2_nrm(vec2         v);

inline vec3 vec3_add(vec3 a, vec3 b) { return (vec3) { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline vec3 vec3_sub(vec3 a, vec3 b) { return (vec3) { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline vec3 vec3_mul(vec3 a, f32  b) { return (vec3) { a.x * b, a.y * b, a.z * b };       }
inline vec3 vec3_div(vec3 a, f32  b) { return (vec3) { a.x / b, a.y / b, a.z / b };       }
inline f32  vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z;          }
inline f32  vec3_sql(vec3         v) { return v.x * v.x + v.y * v.y + v.z + v.z;          }
inline f32  vec3_len(vec3         v) { return sqrtf(v.x * v.x + v.y * v.y + v.z + v.z);   }
inline vec3 vec3_crs(vec3 a, vec3 b) {
    return (vec3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
vec3        vec3_nrm(vec3 v);

inline vec4 vec4_add(vec4 a, vec4 b) { return (vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
inline vec4 vec4_sub(vec4 a, vec4 b) { return (vec4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
inline vec4 vec4_mul(vec4 a, f32  b) { return (vec4) { a.x * b, a.y * b, a.z * b, a.w * b };         }
inline vec4 vec4_div(vec4 a, f32  b) { return (vec4) { a.x / b, a.y / b, a.z / b, a.w / b };         }
inline f32  vec4_dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;         }
inline f32  vec4_sql(vec4         v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;         }
inline f32  vec4_len(vec4         v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);  }
vec4        vec4_nrm(vec4         v);

#endif // BASE_MATH_H
