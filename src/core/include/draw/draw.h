#ifndef DRAW_H
#define DRAW_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "gfx/gfx.h"

#ifdef AP_OPENGL
#include "gfx/opengl/opengl.h"
#endif

// TODO: vec4 for color

#include "draw_rect_batch.h"
#include "draw_polygon.h"
#include "draw_bezier.h"

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
