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

#define SHADER_PATH	"shaders/"
#define TEXTURE_PATH	"images/"
#define MODEL_PATH	"models/"
#define DATA_PATH	"data/"

#define DATA_ENVIRON	DATA_PATH "environment.txt"
#define DATA_WAVEFRONT	DATA_PATH "wavefront.txt"
#define DATA_BULLET	DATA_PATH "bullet.txt"

#define BULLET
//#define MODELS
#define TEXTURE_ALPHA
#define CULL_FACE
#define ALPHA_BLEND

// P
#define CAMERA_V0_POS	glm::vec3(-0.266186, 0.207137, 0.350882)
#define CAMERA_V0_ROT	glm::quat(0.864666, -0.0412326, -0.50014, -0.0227479)
// L
#define CAMERA_V1_POS	glm::vec3(-0.164747, 0.187142, 0.48217)
#define CAMERA_V1_ROT	glm::quat(0.998579, -0.052822, 0.00645501, -0.0026899)
// O
#define CAMERA_V2_POS	glm::vec3(-5.97896, 8.86862, 3.46077)
#define CAMERA_V2_ROT	glm::quat(0.715113, -0.369985, -0.488345, -0.336535)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#define PI		(glm::pi<GLfloat>())

// Different programs
enum {
	PROGRAM_TEXTURE = 0,
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
#define UNIFORM_LIGHT_DIRECTION	"light"
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
	TEXTURE_WHITE = 0,
	TEXTURE_CAMERA,
	//TEXTURE_SPHERE,
	//TEXTURE_FIREMAP,
	TEXTURE_GLOW,
	TEXTURE_CUBE,
	TEXTURE_SKYBOX,
	TEXTURE_GROUND,
	//TEXTURE_GROUND_BUMP,
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