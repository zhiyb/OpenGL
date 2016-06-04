#ifndef GLOBAL_H
#define GLOBAL_H

#include <unordered_map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#define TEXTURE_ALPHA

#define SHADER_PATH	"shaders/"
#define TEXTURE_PATH	"images/"

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#define PI		(glm::pi<GLfloat>())

// Different programs
enum {
	PROGRAM_BACKGROUND = 0,
	PROGRAM_TEXTURED_BASIC,
	PROGRAM_COUNT
};

// Uniform locations
#define UNIFORM_MAT_MV		"mvMatrix"
#define UNIFORM_MAT_MODEL	"modelMatrix"
#define UNIFORM_TIME		"time"
#define UNIFORM_COLOUR		"colour"
#define UNIFORM_SAMPLER		"sampler"

// Attribuate locations
enum {
	ATTRIB_POSITION = 0,
	ATTRIB_TEXCOORD,
	ATTRIB_COUNT
};

// Textures
enum {
	TEXTURE_WHITE = 0,
	TEXTURE_BIRD,
	TEXTURE_PIPE,
	TEXTURE_DEBUG,
	TEXTURE_COUNT
};

struct uniform_t {
	// location = -1: silently ignored
	uniform_t() : id(-1) {}
	uniform_t(int i) : id(i) {}
	operator int() {return id;}
	GLint id;
};
typedef std::unordered_map<std::string, uniform_t> uniformMap;

extern struct program_t {
	uniformMap uniforms;
	GLuint id;
} programs[PROGRAM_COUNT];

extern struct texture_t {
	GLuint id;
	//GLenum type;
	int x, y, n;
} textures[TEXTURE_COUNT];

#endif // GLOBAL_H
