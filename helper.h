#ifndef HELPER_H
#define HELPER_H

struct shader_t {
	GLenum type;
	const char *path;
};

// delete[] after use!
char *readFile(const char *path);

GLuint setupShader(const GLenum type, const char *source);
GLuint setupShaderFromFile(GLenum type, const char *path);
GLuint setupProgram(GLuint program, const GLuint *shaders);
GLuint setupProgramFromFiles(GLuint program, const shader_t *shaders);

GLuint setupPrograms();
GLuint setupTextures();

#endif // HELPER_H
