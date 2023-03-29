#ifndef DRAW_H
#define DRAW_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "gfx/gfx.h"

#ifdef AP_OPENGL
#include "gfx/opengl/opengl.h"
#endif

typedef enum {
    DRAW_FILTER_NEAREST,
    DRAW_FILTER_LINEAR
} draw_filter_type;

#include "draw_rect_batch.h"
#include "draw_polygon.h"
#include "draw_bezier.h"

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
