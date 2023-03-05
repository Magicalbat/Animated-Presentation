#ifndef APP_APP_H
#define APP_APP_H

#include "gfx/gfx.h"
#include "draw/draw.h"
#include "app_pres.h"

typedef struct ap_app {
    gfx_window* win;

    draw_rectb* rectb;
    draw_cbezier* cbezier;
    draw_polygon* poly;

    pres* pres;
} ap_app;

ap_app* app_create(marena* arena, pres* pres, u32 win_width, u32 win_height);
void app_run(marena* arena, ap_app* app);
void app_destroy(ap_app* app);

#endif // APP_APP_H
