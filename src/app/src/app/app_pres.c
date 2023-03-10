#include "app/app_pres.h"

/*
*/

apres* pres_parse(marena* arena, string8 file_path) {
    apres* out = CREATE_ZERO_STRUCT(arena, apres);
    
    marena_temp scratch = marena_scratch_get(&arena, 1);

    string8 file = os_file_read(scratch.arena, file_path);

    marena_scratch_release(scratch);

    return out;
}

void slide_draw(slide_node* slide, ap_app* app) { }
void slide_update(slide_node* slide, f32 delta) { }