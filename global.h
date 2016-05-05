#ifndef GLOBAL_H
#define GLOBAL_H

//#include <map>
#include <unordered_map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#define SHADER_PATH	"./shaders/"
#define TEXTURE_PATH	"./images/"
#define MODEL_PATH	"./models/"

//#define BULLET
//#define MODELS

#ifdef BULLET
#define CAMERA_INIT_POS	glm::vec3(0.f, 0.f, 1.f + arena.scale + 3.f)
#else
#define CAMERA_INIT_POS	glm::vec3(0.f, 1.f, 1.f + 3.f)
#endif
// P
//#define CAMERA_V0_POS	glm::vec3(-1.78653, 0.990645, 1.56153)
//#define CAMERA_V0_ROT	glm::quat(0.93955, 0.0159436, -0.341968, 0.00709854)
#define CAMERA_V0_POS	glm::vec3(-3.30448, 4.24575, 1.883)
#define CAMERA_V0_ROT	glm::quat(0.833449, -0.247321, -0.47099, -0.14956)
// L
#define CAMERA_V1_POS	glm::vec3(0, 1.00613, 2.41957)
#define CAMERA_V1_ROT	glm::quat(0.999848, 0.0174524, 0, 0)
// O
#define CAMERA_V2_POS	glm::vec3(0, 3.06334, 1.30881)
#define CAMERA_V2_ROT	glm::quat(0.920509, -0.390733, 0, 0)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#define PI		(glm::pi<GLfloat>())

// Different programs
enum {
	PROGRAM_BASIC = 0,
	PROGRAM_LIGHTING,
	PROGRAM_TEXTURE,
	PROGRAM_WAVEFRONT,
	PROGRAM_SKYBOX,
	PROGRAM_COUNT
};

// Uniform locations
#define UNIFORM_MVP		"mvpMatrix"
#define UNIFORM_MODEL		"modelMatrix"
#define UNIFORM_NORMAL		"normalMatrix"
#define UNIFORM_AMBIENT		"ambient"
#define UNIFORM_DIFFUSE		"diffuse"
#define UNIFORM_SPECULAR	"specular"
#define UNIFORM_EMISSION	"emission"
#define UNIFORM_SHININESS	"shininess"
#define UNIFORM_VIEWER		"viewer"
#define UNIFORM_LIGHT		"light"
#define UNIFORM_LIGHT_INTENSITY	"lightIntensity"
#define UNIFORM_ENVIRONMENT	"environment"
#define UNIFORM_COLOUR		"colour"
#define UNIFORM_TEXTURED	"textured"

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
	TEXTURE_SKYBOX,
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
	GLuint texture;
	//GLenum type;
	int x, y, n;
} textures[TEXTURE_COUNT];

#endif // GLOBAL_H
