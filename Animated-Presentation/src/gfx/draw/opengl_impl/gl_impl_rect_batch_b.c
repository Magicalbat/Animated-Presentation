#ifdef AP_OPENGL

#if 0
#include "gl_impl.h"

draw_rectb_t* draw_rectb_create(arena_t* arena, u64 capacity) { 
    draw_rectb_t* batch = CREATE_ZERO_STRUCT(arena, batch, draw_rectb_t);

    batch->data = CREATE_ARRAY(arena, draw_rect_t, capacity);
    batch->capacity = capacity;
    batch->size = 0;

	const char* vertex_source = ""
		"#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
		"}";
	const char* fragment_source = ""
		"#version 330 core\n"
		//"out vec4 FragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_FragColor = vec4(0.2f, 0.8f, 0.5f, 1.0f);\n"
		"}";
    batch->gl.shader_program = gl_impl_create_shader_program(vertex_source, fragment_source);
 
    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);

    batch->gl.vertex_buffer = gl_impl_create_buffer(
        GL_ARRAY_BUFFER, capacity * sizeof(draw_rect_t), NULL, GL_DYNAMIC_DRAW
    );

    u32* indices = CREATE_ARRAY(arena, u32, capacity * 6);
    for (int i = 0; i < capacity; i++) {
        indices[i * 6 + 0] = i * 4 + 0;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        
        indices[i * 6 + 3] = i * 4 + 0;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }

    batch->gl.index_buffer = gl_impl_create_buffer(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * capacity * 6, indices, GL_STATIC_DRAW
    );

    arena_pop(arena, sizeof(u32) * capacity * 6);

    return batch;
}
void draw_rectb_destroy(draw_rectb_t* batch) {
    glDeleteProgram(batch->gl.shader_program);
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.index_buffer);
}

void draw_rectb_push(draw_rectb_t* batch, rect_t rect) {
    if (batch->size < batch->capacity) {
        batch->data[batch->size++] = (draw_rect_t){
            .bottom_left  = (vec2_t){ rect.x,          rect.y          },
            .top_left     = (vec2_t){ rect.x,          rect.y + rect.h },
            .top_right    = (vec2_t){ rect.x + rect.w, rect.y + rect.h },
            .bottom_right = (vec2_t){ rect.x + rect.w, rect.y          }
        };
    } else {
        draw_rectb_flush(batch);
        draw_rectb_push(batch, rect);
    }
}
void draw_rectb_flush(draw_rectb_t* batch) {
    glBindVertexArray(batch->gl.vertex_array);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->size * sizeof(draw_rect_t), batch->data);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->gl.index_buffer);
    glUseProgram(batch->gl.shader_program);

    glDrawElements(GL_TRIANGLES, batch->size * 6, GL_UNSIGNED_INT, NULL);
    
    glDisableVertexAttribArray(0);

    batch->size = 0;
}

#endif // 0
#endif // AP_OPENGL