#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>

#include <GL/glew.h>

#define SHADER_PATH	"./shaders/"
#define TEXTURE_PATH	"./images/"
#define MODEL_PATH	"./models/"

//#define BULLET
//#define MODELS

#ifdef BULLET
#define CAMERA_INIT_POS	glm::vec3(0.f, 0.f, 1.f + arena.scale + 3.f)
#else
#define CAMERA_INIT_POS	glm::vec3(0.f, 0.f, 1.f + 3.f)
#endif
#define CAMERA_V0_POS	CAMERA_INIT_POS
#define CAMERA_V0_ROT	glm::quat()
#define CAMERA_V1_POS	glm::vec3(0, 1.00613, 2.41957)
#define CAMERA_V1_ROT	glm::quat(0.999848, 0.0174524, 0, 0)
#define CAMERA_V2_POS	glm::vec3(0, 3.06334, 1.30881)
#define CAMERA_V2_ROT	glm::quat(0.920509, -0.390733, 0, 0)

#define PI		(glm::pi<GLfloat>())

// Different programs
enum {
	PROGRAM_BASIC = 0,
	PROGRAM_LIGHTING,
	PROGRAM_TEXTURE,
	PROGRAM_WAVEFRONT,
	PROGRAM_COUNT
};

// Uniform locations
enum {
	UNIFORM_MVP = 0,
	UNIFORM_MODEL,
	UNIFORM_NORMAL,
	UNIFORM_AMBIENT,
	UNIFORM_DIFFUSE,
	UNIFORM_SPECULAR,
	UNIFORM_SHININESS,
	UNIFORM_VIEWER,
	UNIFORM_LIGHT,
	UNIFORM_COLOUR,
	UNIFORM_TEXTURED,
	UNIFORM_COUNT
};

// Attribuate locations
enum {
	ATTRIB_POSITION = 0,
	ATTRIB_NORMAL,
	ATTRIB_TEXCOORD,
	ATTRIB_COUNT
};

// Textures
enum {
	TEXTURE_SPHERE,
	TEXTURE_S2,
	TEXTURE_CUBE,
	TEXTURE_COUNT
};

struct program_t {
	// Uniform locations
	GLint uniforms[UNIFORM_COUNT];
	GLuint id;
};
extern program_t programs[PROGRAM_COUNT];

struct texture_t {
	GLuint texture;
	int x, y, n;
};
extern texture_t textures[TEXTURE_COUNT];

#endif // GLOBAL_H
