#ifndef X
# define X(ret, name, args)
#endif

X(void, glGenBuffers, (GLsizei n, GLuint * buffers))
X(void, glBindBuffer, (GLenum target, GLuint buffer)) 
X(void, glBufferData, (GLenum target, GLsizeiptr size, const void * data, GLenum usage)) 
X(void, glDeleteBuffers, (GLsizei n, const GLuint * buffers)) 

X(void, glGenVertexArrays, (GLsizei n, GLuint *arrays))
X(void, glBindVertexArray, (GLuint array))

X(void, glEnableVertexAttribArray, (GLuint index))
X(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer))

X(GLuint, glCreateShader, (GLenum shaderType))
X(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar **string, const GLint *length))
X(void, glCompileShader, (GLuint shader))
X(void, glAttachShader, (GLuint program, GLuint shader))
X(void, glDeleteShader, (GLuint shader))
X(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint *params))
X(void, glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))

X(GLuint, glCreateProgram, (void))
X(void, glLinkProgram, (GLuint program))
X(void, glUseProgram, (GLuint program))
X(void, glDeleteProgram, (GLuint program))
X(void, glGetProgramiv, (GLuint program, GLenum pname, GLint *params))
X(void, glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))

X(void, glDebugMessageCallback, (GLDEBUGPROC callback, const void *userParam))
