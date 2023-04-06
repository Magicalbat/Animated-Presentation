#include "plugin.h"

AP_EXPORT void plugin_init(marena* arena, app_app* app) {
    rectangle_obj_init(arena, app);
    image_obj_init(arena, app);
    bezier_obj_init(arena, app);
    polygon_obj_init(arena, app);
}
