#ifdef AP_OPENGL

#include "gfx/gfx.h"
#include "gfx/opengl/opengl.h"

#include "gfx/draw/draw_rect_batch.h"

draw_rect_batch_t* draw_rect_batch_create(arena_t* arena, u64 capacity) { 
    draw_rect_batch_t* batch = CREATE_ZERO_STRUCT(arena, batch, draw_rect_batch_t);

    batch->data = CREATE_ARRAY(arena, draw_rect_t, capacity);
    batch->capacity = capacity;
    batch->size = 0;

	const char* vertex_shader_source = ""
		"#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
		"}";
	const char* fragment_shader_source = ""
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"\n"
		"void main()\n"
		"{ \n"
		"    FragColor = vec4(0.2f, 0.8f, 0.5f, 1.0f);\n"
		"}";
 
	u32 vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	
	i32 success = GL_TRUE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		char info_log[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		printf("Failed to compile vertex shader: %s", info_log);
	}
	
	u32 fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char info_log[512];
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		printf("Failed to compile fragment shader: %s", info_log);
	}

	batch->gl.shader_program = glCreateProgram();
	glAttachShader(batch->gl.shader_program, vertex_shader);
	glAttachShader(batch->gl.shader_program, fragment_shader);
	glLinkProgram(batch->gl.shader_program);

	glGetProgramiv(batch->gl.shader_program, GL_LINK_STATUS, &success);
	if(!success) {
		char info_log[512];
		glGetProgramInfoLog(batch->gl.shader_program, 512, NULL, info_log);
		printf("Failed to link shader: %s", info_log);
	}
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

    glGenVertexArrays(1, &batch->gl.vertex_array);
    glBindVertexArray(batch->gl.vertex_array);

    glGenBuffers(1, &batch->gl.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, batch->gl.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(draw_rect_t), NULL, GL_DYNAMIC_DRAW);

    u32* indices = CREATE_ARRAY(arena, u32, capacity * 6);
    for (int i = 0; i < capacity; i++) {
        indices[i * 6 + 0] = i * 4 + 0;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        
        indices[i * 6 + 3] = i * 4 + 0;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }

    glGenBuffers(1, &batch->gl.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->gl.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * capacity * 6, indices, GL_STATIC_DRAW);
    

    arena_pop(arena, sizeof(u32) * capacity * 6);

    return batch;
}
void draw_rect_batch_destroy(draw_rect_batch_t* batch) {
    glDeleteProgram(batch->gl.shader_program);
    glDeleteVertexArrays(1, &batch->gl.vertex_array);
    glDeleteBuffers(1, &batch->gl.vertex_buffer);
    glDeleteBuffers(1, &batch->gl.index_buffer);
}

void draw_rect_batch_push(draw_rect_batch_t* batch, rect_t rect) {
    if (batch->size < batch->capacity) {
        batch->data[batch->size++] = (draw_rect_t){
            .bottom_left  = (vec2_t){ rect.x, rect.y },
            .top_left     = (vec2_t){ rect.x, rect.y + rect.h },
            .top_right    = (vec2_t){ rect.x + rect.w, rect.y + rect.h },
            .bottom_right = (vec2_t){ rect.x + rect.w, rect.y }
        };
    } else {
        draw_rect_batch_flush(batch);
        draw_rect_batch_push(batch, rect);
    }
}
void draw_rect_batch_flush(draw_rect_batch_t* batch) {
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

#endif // AP_OPENGL
