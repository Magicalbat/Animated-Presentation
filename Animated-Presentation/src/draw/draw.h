#ifndef DRAW_H
#define DRAW_H

#include "gfx/gfx.h"

#ifdef AP_OPENGL
#include "gfx/opengl/opengl.h"
#endif

// TODO: Make uniforms update on window resize

#include "draw_rect_batch.h"
#include "draw_polygon.h"
#include "draw_bezier.h"

#endif // DRAW_H
