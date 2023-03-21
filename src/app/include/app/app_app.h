#ifndef APP_APP_H
#define APP_APP_H

#ifdef __cplusplus
extern "C" { 
#endif

#include "gfx/gfx.h"
#include "draw/draw.h"

typedef struct app_pres app_pres;

typedef struct app_app {
    gfx_window* win;
    f32 ref_width;
    f32 ref_height;
    f32 aspect_ratio;
    f32 win_mat[16];

    draw_rectb* rectb;
    draw_cbezier* cbezier;
    draw_polygon* poly;

    app_pres* pres;

    struct {
        marena* arena;
        string8_registry file_reg;
    } temp;
} app_app;

app_app* app_create(marena* arena, string8 pres_path, u32 win_width, u32 win_height);
void app_run(marena* arena, app_app* app);
void app_destroy(app_app* app);

#ifdef __cplusplus
}
#endif

#endif // APP_APP_H
