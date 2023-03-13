#ifndef APP_APP_H
#define APP_APP_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "gfx/gfx.h"
#include "draw/draw.h"
#include "app_pres.h"

typedef struct ap_app {
    gfx_window* win;

    draw_rectb* rectb;
    draw_cbezier* cbezier;
    draw_polygon* poly;

    apres* pres;
} ap_app;

ap_app* app_create(marena* arena, string8 pres_path, u32 win_width, u32 win_height);
void app_run(marena* arena, ap_app* app);
void app_destroy(ap_app* app);

#ifdef __cplusplus
}
#endif

#endif // APP_APP_H
