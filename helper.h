#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include "global.h"

struct shader_t {
	GLenum type;
	const char *path;
};

std::string basename(std::string &path);
std::istream &operator>>(std::istream &stream, glm::vec3 &vec);
std::istream &operator>>(std::istream &stream, glm::vec4 &vec);

// delete[] after use!
char *readFile(const char *path);

bool checkError(const char *msg);

GLuint setupShader(const GLenum type, const char *source);
GLuint setupShaderFromFile(GLenum type, const char *path);
GLuint setupProgram(GLuint program, const GLuint *shaders);
GLuint setupProgramFromFiles(GLuint program, const shader_t *shaders);

GLuint setupPrograms();
GLuint setupTextures();

texture_t loadTexture(const char *path);

#endif // HELPER_H
