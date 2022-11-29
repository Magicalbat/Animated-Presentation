#ifdef AP_OPENGL

#include "draw.h"


draw_rect_batch_t* draw_rect_batch_create(arena_t* arena, u64 capacity) { return NULL; }
void draw_rect_batch_destroy(arena_t* arena, u64 capacity) { }

void draw_rect_batch_push(draw_rect_batch_t* batch, draw_rect_t rect) { }
void draw_rect_batch_flush(draw_rect_batch_t* batch) { }

#endif // AP_OPENGL
