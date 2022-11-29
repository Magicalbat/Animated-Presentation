#include "os/os.h"
#include "base/base.h"
#include "gfx/gfx.h"

#include "gfx/opengl/opengl.h"

// https://www.khronos.org/opengl/wiki/OpenGL_Error
void opengl_message_callback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

int main(int argc, char** argv) {
    os_main_init(argc, argv);

    arena_t* perm_arena = arena_create(KB(16));

    gfx_window_t* win = gfx_win_create(perm_arena, 320, 180, str8_lit("Test window"));
    gfx_win_make_current(win);
    opengl_load_functions(win);

    printf("GL Vender: %s\n",   glGetString(GL_VENDOR));
	printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL Version: %s\n",  glGetString(GL_VERSION));

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(opengl_message_callback, 0);

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

	u32 shader_program;
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if(!success) {
		char info_log[512];
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		printf("Failed to link shader: %s", info_log);
	}
	
	glUseProgram(shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	f32 vertices[6] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.0f,  0.5f
	};

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	u32 vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    // TODO: Better frame independence
    u64 time_prev = os_now_microseconds();

    while (!win->info.should_close) {
        u64 time_now = os_now_microseconds();
        f32 delta = (f32)(time_now - time_prev) / 1000000.0f;

        gfx_win_process_events(win);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        for (u32 i = 0; i < 3; i++) {
            vertices[i * 2] += delta * 0.15f;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);

        glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

        gfx_win_swap_buffers(win);

        time_prev = time_now;
    }

    gfx_win_destroy(win);

    arena_free(perm_arena);

    os_main_quit();

    return 0;
}
