#include "base/base_defs.h"

#ifdef AP_OPENGL

#include "draw/opengl_impl/gl_impl.h"

static const char* vert_source;
static const char* frag_source;
static const char* frag_texture_opts[];

draw_rectb* draw_rectb_create(marena* arena, f32* win_mat, u32 capacity, u32 max_textures, u32 channels) { 
    draw_rectb* batch = CREATE_ZERO_STRUCT(arena, draw_rectb);

    batch->win_mat = win_mat;

    batch->data = CREATE_ARRAY(arena, draw_rectb_rect, capacity);
    batch->capacity = capacity;

    if (channels == 0 || channels > 4) {
        log_errorf("Invalid number of channels %u for rect batch, defaulting to 4", channels);
        batch->channels = 4;
    } else {
        batch->channels = channels;
    }
    
    {
        marena_temp scratch = marena_scratch_get(&arena, 1);

        const char* frag_opt = frag_texture_opts[batch->channels];
        u64 frag_source_size = strlen(frag_source) + strlen(frag_opt) + 1;

        u8* computed_frag = CREATE_ZERO_ARRAY(scratch.arena, u8, frag_source_size);
        snprintf((char*)computed_frag, frag_source_size, frag_source, frag_opt);

        batch->gl.shader_program = gl_impl_create_shader_program(vert_source, (char*)computed_frag); 
        glUseProgram(batch->gl.shader_program);

        marena_scratch_release(scratch);
    }

    batch->gl.win_mat_loc = glGetUniformLocation(batch->gl.shader_program, "u_win_mat");
    batch->gl.texture_loc = glGetUniformLocation(batch->gl.shader_program, "u_texture");


#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);
#endif

    f32 pos_pattern[] = {
        0, 1,
        1, 1,
        0, 0,
        1, 1,
        0, 0,
        1, 0  
    };
    
    batch->gl.pos_pattern_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(pos_pattern), &pos_pattern[0], GL_STATIC_DRAW
    );
    
    batch->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, sizeof(draw_rectb_rect) * capacity, NULL, GL_DYNAMIC_DRAW
    );
    
    batch->max_textures = max_textures;
    batch->tex_rects = CREATE_ZERO_ARRAY(arena, rect, max_textures);
    batch->img_rects = CREATE_ZERO_ARRAY(arena, rect, max_textures);

    batch->temp.filter_type = DRAW_FILTER_NEAREST;
    batch->temp.arena = marena_create(&(marena_desc){ .desired_max_size = MiB(64) });
    batch->temp.imgs = CREATE_ZERO_ARRAY(batch->temp.arena, image, max_textures);

    u32* color = (u32*)marena_push(batch->temp.arena, sizeof(u32));
    *color = 0xffffffff;
    draw_rectb_add_tex(batch, (image){
        .width = 1,
        .height = 1,
        .channels = 4,
        .data = (u8*)color
    });
    
    return batch;
}
void draw_rectb_destroy(draw_rectb* batch) {
    glDeleteProgram(batch->gl.shader_program);

#ifndef __EMSCRIPTEN__
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
#endif
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.pos_pattern_buffer);

    glDeleteTextures(1, &batch->gl.texture);
}

void draw_rectb_set_filter(draw_rectb* batch, draw_filter_type filter_type) {
    batch->temp.filter_type = filter_type;
}
u32 draw_rectb_add_tex(draw_rectb* batch, image img) {
    u32 id = batch->num_textures;
    if (id >= batch->max_textures) {
        log_errorf("Cannot create any more textures for rectangle batch, max is %u", batch->max_textures);
        return -1;
    } else {
        batch->num_textures++;
    }

    batch->temp.imgs[id] = img;
    batch->img_rects[id] = (rect){ .w = (f32)img.width, .h = (f32)img.height };

    return id;
}
u32 draw_rectb_create_tex(draw_rectb* batch, string8 image_file, vec2* dim) {
    image img = parse_image(batch->temp.arena, image_file, 4);
    
    if (!img.valid) {
        return -1;
    }
    
    if (dim != NULL) {
        dim->x = img.width;
        dim->y = img.height;
    }

    u32 id = draw_rectb_add_tex(batch, img);

    return id;
}
u32 draw_rectb_load_tex(draw_rectb* batch, string8 file_path, vec2* dim) {
    string8 file = os_file_read(batch->temp.arena, file_path);
    if (file.size == 0) {
        return -1;
    }
    
    image img = parse_image(batch->temp.arena, file, 4);
        
    if (!img.valid) {
        return -1;
    }
    
    if (dim != NULL) {
        dim->x = img.width;
        dim->y = img.height;
    }

    u32 id = draw_rectb_add_tex(batch, img);

    return id;
}
void draw_rectb_finalize_textures(draw_rectb* batch) {
    rect boundary = rect_pack(batch->img_rects, batch->num_textures);
    
    batch->texture_boundary = boundary;

    for (u32 i = 0; i < batch->num_textures; i++) {
        batch->tex_rects[i] = batch->img_rects[i];

        batch->tex_rects[i].x /= boundary.w;
        batch->tex_rects[i].y /= boundary.h;
        batch->tex_rects[i].w /= boundary.w;
        batch->tex_rects[i].h /= boundary.h;
    }
    
    glGenTextures(1, &batch->gl.texture);
    glBindTexture(GL_TEXTURE_2D, batch->gl.texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    if (batch->temp.filter_type == DRAW_FILTER_LINEAR) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    u32 in_type = GL_RGBA8;
    u32 color_type = GL_RGBA;
    switch (batch->channels) {
        case 1: { in_type = GL_R8; color_type = GL_RED; break; }
        case 3: { in_type = GL_RGB8; color_type = GL_RGB; break; }
        case 4: { in_type = GL_RGBA8; color_type = GL_RGBA; break; }
        default: break;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, in_type, (GLsizei)boundary.w, (GLsizei)boundary.h, 0, color_type, GL_UNSIGNED_BYTE, NULL);

    for (u32 i = 0; i < batch->num_textures; i++) {
        image* img = &batch->temp.imgs[i];
        rect* rect = &batch->img_rects[i];

        glTexSubImage2D(GL_TEXTURE_2D, 0,
            (GLint)rect->x, (GLint)rect->y,
            (GLsizei)rect->w, (GLsizei)rect->h,
            color_type, GL_UNSIGNED_BYTE, img->data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    marena_destroy(batch->temp.arena);
}

void draw_rectb_push_ex(draw_rectb* batch, rect draw_rect, vec4d col, i32 tex_id, rect tex_rect) {
    if (batch->size >= batch->capacity)
        draw_rectb_flush(batch);

    rect tr = batch->tex_rects[tex_id];
    if (tex_rect.x != 0) {
        tr.x += (tex_rect.x / batch->img_rects[tex_id].w) * tr.w;
        tr.y += (tex_rect.y / batch->img_rects[tex_id].h) * tr.h;
        
        tr.w = (tex_rect.w / batch->img_rects[tex_id].w) * tr.w;
        tr.h = (tex_rect.h / batch->img_rects[tex_id].h) * tr.h;
    }
    
    batch->data[batch->size++] = (draw_rectb_rect){
        .draw_rect = draw_rect,
        .col = (vec4){ col.x, col.y, col.z, col.w },
        .tex_rect = tr//(rect){ 0, 0, 1, 1 }
    };
}
void draw_rectb_push(draw_rectb* batch, rect draw_rect, vec4d col) {
    draw_rectb_push_ex(batch, draw_rect, col, 0, (rect){ 0.0f, 0.0f, 1.0f, 1.0f }); 
}

void draw_rectb_flush(draw_rectb* batch) {
    glUseProgram(batch->gl.shader_program);
    glUniformMatrix4fv(batch->gl.win_mat_loc, 1, GL_FALSE, batch->win_mat);

#ifndef __EMSCRIPTEN__
    glBindVertexArray(batch->gl.vertex_array);
#endif

    glBindTexture(GL_TEXTURE_2D, batch->gl.texture);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(batch->gl.texture_loc, 0);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.pos_pattern_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, draw_rect)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, col)));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(draw_rectb_rect), (void*)(offsetof(draw_rectb_rect, tex_rect)));
    
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * sizeof(draw_rectb_rect), batch->data);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)batch->size);
    
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glVertexAttribDivisor(3, 0);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    batch->size = 0;
}

static const char* vert_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "attribute vec2 a_pos_pattern;"
    "attribute vec4 a_quad;"
    "attribute vec4 a_col;"
    "attribute vec4 a_tex_rect;"
    "varying vec4 col;"
    "varying vec2 uv;"
#else
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_pos_pattern;"
    "layout (location = 1) in vec4 a_quad;"
    "layout (location = 2) in vec4 a_col;"
    "layout (location = 3) in vec4 a_tex_rect;"
    "out vec4 col;"
    "out vec2 uv;"
#endif
    "uniform mat4 u_win_mat;"
    "void main() {"
    "    col = a_col;"
    "    uv = a_tex_rect.xy + a_tex_rect.zw * a_pos_pattern;"
    "    vec2 pos = a_quad.xy + a_quad.zw * a_pos_pattern;"
    "    gl_Position = vec4(pos, 0, 1) * u_win_mat;"
    "\n}";
        
static const char* frag_source = ""
#ifdef __EMSCRIPTEN__
    "precision mediump float;"
    "varying vec4 col;"
    "varying vec2 uv;"
    "uniform sampler2D u_texture;"
    "void main() {"
    "    gl_FragColor = %s * col;"
    "\n}";
#else
    "#version 400 core\n"
    "layout (location = 0) out vec4 out_col;"
    "in vec4 col;"
    "in vec2 uv;"
    "uniform sampler2D u_texture;"
    "void main() {"
    "    out_col = %s * col;"
    "\n}";
#endif

static const char* frag_texture_opts[] = {
#ifdef __EMSCRIPTEN__
    NULL,
    "vec4(1, 1, 1, texture2D(u_texture, uv).r)",
    NULL,
    "texture2D(u_texture, uv)",
    "texture2D(u_texture, uv)"
#else
    NULL,
    "vec4(1, 1, 1, texture(u_texture, uv).r)",
    NULL,
    "texture(u_texture, uv)",
    "texture(u_texture, uv)"
#endif
};

#endif // AP_OPENGL
