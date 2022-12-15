#ifndef DRAW_RECT_BATCH_H
#define DRAW_RECT_BATCH_H

// TODO: benchmark this vs instancing
typedef struct {
    vec2_t bottom_left;
    vec2_t top_left;
    vec2_t top_right;
    vec2_t bottom_right;
} draw_rect_t;

typedef struct {
    draw_rect_t* data;
    u64 capacity;
    u64 size;

    #ifdef AP_OPENGL
    struct {
        u32 shader_program;

        u32 vertex_array;
        u32 vertex_buffer;
        u32 index_buffer;
    } gl;
    #endif
} draw_rectb_t;

draw_rectb_t* draw_rectb_create(arena_t* arena, u64 capacity);
void          draw_rectb_destroy(draw_rectb_t* batch);

// These will draw rects to the screen
void draw_rectb_push(draw_rectb_t* batch, rect_t rect);
void draw_rectb_flush(draw_rectb_t* batch);

#endif // DRAW_RECT_BATCH_H
