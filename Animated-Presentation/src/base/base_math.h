#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <math.h>

#include "base.h"

typedef struct vec2 { float x, y;       } vec2_t;
typedef struct vec3 { float x, y, z;    } vec3_t;
typedef struct vec4 { float x, y, z, w; } vec4_t;

vec2_t vec2_add( vec2_t a, vec2_t b );
vec2_t vec2_sub( vec2_t a, vec2_t b );
vec2_t vec2_mul( vec2_t a, float  b );
vec2_t vec2_div( vec2_t a, float  b );
float  vec2_dot( vec2_t a, vec2_t b ); 
float  vec2_sql( vec2_t v           );
float  vec2_len( vec2_t v           );
vec2_t vec2_nrm( vec2_t v           );
vec2_t vec2_prp( vec2_t v           );

vec3_t vec3_add( vec3_t a, vec3_t b );
vec3_t vec3_sub( vec3_t a, vec3_t b );
vec3_t vec3_mul( vec3_t a, float  b );
vec3_t vec3_div( vec3_t a, float  b );
vec3_t vec3_crs( vec3_t a, vec3_t b );
float  vec3_dot( vec3_t a, vec3_t b ); 
float  vec3_sql( vec3_t v           );
float  vec3_len( vec3_t v           );
vec3_t vec3_nrm( vec3_t v           );

vec4_t vec4_add( vec4_t a, vec4_t b );
vec4_t vec4_sub( vec4_t a, vec4_t b );
vec4_t vec4_mul( vec4_t a, float  b );
vec4_t vec4_div( vec4_t a, float  b );
float  vec4_dot( vec4_t a, vec4_t b ); 
float  vec4_sql( vec4_t v           );
float  vec4_len( vec4_t v           );
vec4_t vec4_nrm( vec4_t v           );

#endif