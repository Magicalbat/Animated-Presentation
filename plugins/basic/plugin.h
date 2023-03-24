#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ap_core.h"
#include "app/app.h"

void rectangle_obj_init(marena* arena, app_app* app);
void image_obj_init(marena* arena, app_app* app);
void bezier_obj_init(marena* arena, app_app* app);
void polygon_obj_init(marena* arena, app_app* app);

#ifdef __cplusplus
}
#endif

#endif // PLUGIN_H
