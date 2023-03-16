#ifndef PLUGIN_H
#define PLUGIN_H

#include "ap_core.h"
#include "app/app.h"

void rectangle_obj_init(marena* arena, ap_app* app);
void image_obj_init(marena* arena, ap_app* app);
void bezier_obj_init(marena* arena, ap_app* app);

#endif // PLUGIN_H