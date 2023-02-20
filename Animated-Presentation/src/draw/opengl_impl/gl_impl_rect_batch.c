#ifdef AP_OPENGL

#include "gl_impl.h"

static const char* vert_source;
static const char* frag_source;

draw_rectb* draw_rectb_create(marena* arena, gfx_window* win, u32 capacity) { 
    draw_rectb* batch = CREATE_ZERO_STRUCT(arena, draw_rectb);

    batch->data = CREATE_ARRAY(arena, draw_rectb_rect, capacity);
    batch->capacity = capacity;
    
    batch->gl.shader_program = gl_impl_create_shader_program(vert_source, frag_source);

    glUseProgram(batch->gl.shader_program);
    
    u32 win_mat_loc = glGetUniformLocation(batch->gl.shader_program, "u_win_mat");
    f32 win_mat[] = {
        2.0f / ((f32)win->width), 0,
        0, 2.0f / -((f32)win->height)
    };
    glUniformMatrix2fv(win_mat_loc, 1, GL_FALSE, &win_mat[0]);

    u32 textures_loc = glGetUniformLocation(batch->gl.shader_program, "u_textures");
    i32 tex_indices[RECTB_MAX_TEXS];
    for (u32 i = 0; i < RECTB_MAX_TEXS; i++) {
        tex_indices[i] = i;
    }
    glUniform1iv(textures_loc, RECTB_MAX_TEXS, tex_indices);

#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);
#endif

    batch->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(draw_rectb_rect) * capacity, NULL, GL_DYNAMIC_DRAW
    );
    f32 pos_pattern[] = {
        0, 1,
        1, 1,
        0, 0,
        1, 0
    };
    
    batch->gl.pos_pattern_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(pos_pattern), &pos_pattern[0], GL_STATIC_DRAW
    );

    draw_rectb_create_tex(batch, STR8_LIT("test_img.png"));
    //u32 color = 0xffffffff;
    //draw_rectb_add_tex(batch, (image){
    //    .width = 1,
    //    .height = 1,
    //    .channels = 4,
    //    .data = (u8*)&color
    //});
    
    return batch;
}
void draw_rectb_destroy(draw_rectb* batch) {
    glDeleteProgram(batch->gl.shader_program);
#ifndef __EMSCRIPTEN__
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
#endif
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.pos_pattern_buffer);

    for (u32 i = 0; i < RECTB_MAX_TEXS; i++) {
        if (batch->textures[i].active) {
            glDeleteTextures(1, &batch->textures[i].gl.tex);
        }
    }
}

u32 draw_rectb_add_tex(draw_rectb* batch, image img) {
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    u32 color_type = img.channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, color_type, img.width, img.height, 0, color_type, GL_UNSIGNED_BYTE, img.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    u32 id = -1;
    for (u32 i = 0; i < RECTB_MAX_TEXS; i++) {
        if (!batch->textures[i].active) {
            id = i;
            break;
        }
    }
    
    if (id == -1) {
        log_errorf("Draw rect batch ran out of textures, max is %u", RECTB_MAX_TEXS);
        glDeleteTextures(1, &texture);
        return -1;
    }

    batch->textures[id] = (draw_rectb_tex){
        .active = true,
        .width = img.width,
        .height = img.height,
        .gl.tex = texture
    };
    
    return id;
}
u32 draw_rectb_create_tex(draw_rectb* batch, string8 file_path) {
    marena_temp scratch = marena_scratch_get(NULL, 0);

    string8 file = os_file_read(scratch.arena, file_path);
    if (file.size == 0) {
        marena_temp_end(scratch);
        return -1;
    }
    
    image img = parse_image(scratch.arena, file);
        
    if (!img.valid) {
        marena_temp_end(scratch);
        return -1;
    }

    u32 id = draw_rectb_add_tex(batch, img);

    marena_scratch_release(scratch);

    return id;
}
void draw_rectb_destroy_tex(draw_rectb* batch, u32 tex_id) {
    if (tex_id >= RECTB_MAX_TEXS || !batch->textures[tex_id].active) {
        log_errorf("Cannot destroy draw rect batch texture id %u", tex_id);
        return;
    }

    glDeleteTextures(1, &batch->textures[tex_id].gl.tex);
    
    batch->textures[tex_id] = (draw_rectb_tex){ 0 };
}

void draw_rectb_push_ex(draw_rectb* batch, rect draw_rect, vec3 col, i32 tex_id, rect tex_rect) {
    if (batch->size >= batch->capacity)
        draw_rectb_flush(batch);
    
    batch->data[batch->size++] = (draw_rectb_rect){
        .draw_rect = draw_rect,
        .col = col,
        .tex_id = (f32)tex_id,
        .tex_rect = tex_rect
    };
}
void draw_rectb_push(draw_rectb* batch, rect draw_rect, vec3 col) {
    draw_rectb_push_ex(batch, draw_rect, col, 0, (rect){ 0.0f, 0.0f, 1.0f, 1.0f }); 
}

void draw_rectb_flush(draw_rectb* batch) {
    glUseProgram(batch->gl.shader_program);

#ifndef __EMSCRIPTEN__
    glBindVertexArray(batch->gl.vertex_array);
#endif

    for (u32 i = 0; i < RECTB_MAX_TEXS; i++) {
        if (batch->textures[i].active) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, batch->textures[i].gl.tex);
        }
    }
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.pos_pattern_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, draw_rect)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, col)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, tex_id)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, tex_rect)));
    
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * sizeof(draw_rectb_rect), batch->data);
    
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, (GLsizei)batch->size);

    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glVertexAttribDivisor(3, 0);
    glVertexAttribDivisor(4, 0);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    batch->size = 0;
}

static const char* vert_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "attribute vec2 a_pos_pattern;"
    "attribute vec4 a_quad;"
    "attribute vec3 a_col;"
    "attribute float a_tex_id;"
    "attribute vec4 a_tex_rect;"
    "varying vec4 col;"
    "varying vec2 uv;"
    "varying float tex_id;"
#else
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos_pattern;"
    "layout (location = 1) in vec4 a_quad;"
    "layout (location = 2) in vec3 a_col;"
    "layout (location = 3) in float a_tex_id;"
    "layout (location = 4) in vec4 a_tex_rect;"
    "out vec4 col;"
    "out vec2 uv;"
    "out float tex_id;"
#endif
    "uniform mat2 u_win_mat;"
    "void main() {"
    "    col = vec4(a_col, 1);"
    "    uv = a_tex_rect.xy + a_tex_rect.zw * a_pos_pattern;"
    "    tex_id = a_tex_id;"
    "    vec2 pos = a_quad.xy + a_quad.zw * a_pos_pattern;"
    "    gl_Position = vec4((pos * u_win_mat) + vec2(-1, 1), 0, 1);"
    "\n}";
        
static const char* frag_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "varying vec4 col;"
    "varying vec2 uv;"
    "varying float tex_id;"
    "uniform sampler2D u_textures[" STRINGIFY(RECTB_MAX_TEXS) "];"
    "void main() {"
    "    int id = int(tex_id);"
    "    gl_FragColor = texture2D(u_textures[0], uv) * col;"
    "\n}";
#else
    "#version 400 core\n"
    "layout (location = 0) out vec4 out_col;"
    "in vec4 col;"
    "in vec2 uv;"
    "in float tex_id;"
    "uniform sampler2D u_textures[" STRINGIFY(RECTB_MAX_TEXS) "];"
    "void main() {"
    "    int id = int(tex_id);"
    "    out_col = texture(u_textures[id], uv) * col;"
    "\n}";
#endif


#endif // AP_OPENGL
