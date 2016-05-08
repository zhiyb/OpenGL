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

#include <btBulletDynamicsCommon.h>

#define SHADER_PATH	"shaders/"
#define TEXTURE_PATH	"images/"
#define MODEL_PATH	"models/"
#define DATA_PATH	"data/"

#define DATA_ENVIRON	DATA_PATH "environment.txt"
#define DATA_MODELS	DATA_PATH "models.txt"
#define DATA_BULLET	DATA_PATH "bullet.txt"
#define DATA_RECORDS	DATA_PATH "records.txt"
#define DATA_OBJECTS	DATA_PATH "objects.txt"

#define MAX_LIGHTS	5
#define LIGHT_ENV	"ENV"
#define LIGHT_CAMERA	"CAM"

//#define SUBMISSION
#define BULLET
//#define MODELS
#define TEXTURE_ALPHA
#define CULL_FACE
#define ALPHA_BLEND
#define SHADOW_TEXTURE_SIZE	2048

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#define PI		(glm::pi<GLfloat>())

// Different programs
enum {
	PROGRAM_TEXTURE_LIGHTING = 0,
	PROGRAM_TEXTURE_LIGHTING_SHADOW,
	PROGRAM_WAVEFRONT,
	PROGRAM_SHADOW,
	PROGRAM_TEXTURE_BASIC,
	PROGRAM_COUNT
};

// Uniform locations
#define UNIFORM_MAT_MVP		"mvpMatrix"
#define UNIFORM_MAT_MODEL	"modelMatrix"
#define UNIFORM_MAT_NORMAL	"normalMatrix"
#define UNIFORM_MAT_SHADOW	"shadowMatrix"
#define UNIFORM_AMBIENT		"material.ambient"
#define UNIFORM_DIFFUSE		"material.diffuse"
#define UNIFORM_SPECULAR	"material.specular"
#define UNIFORM_EMISSION	"material.emission"
#define UNIFORM_SHININESS	"material.shininess"
#define UNIFORM_VIEWER		"viewer"
#define UNIFORM_COLOUR		"colour"
#define UNIFORM_SAMPLER		"sampler"
#define UNIFORM_SAMPLER_SHADOW	"shadowSampler"

#define U_LIGHT_ENABLED(i)	(std::string("lights[" + std::to_string(i) + "].enabled").c_str())
#define U_LIGHT_AMBIENT(i)	(std::string("lights[" + std::to_string(i) + "].ambient").c_str())
#define U_LIGHT_COLOUR(i)	(std::string("lights[" + std::to_string(i) + "].colour").c_str())
#define U_LIGHT_POSITION(i)	(std::string("lights[" + std::to_string(i) + "].position").c_str())
#define U_LIGHT_ATTENUATION(i)	(std::string("lights[" + std::to_string(i) + "].attenuation").c_str())
#define U_LIGHT_SHADOW(i)	(std::string("lights[" + std::to_string(i) + "].shadow").c_str())

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
	GLuint id;
	//GLenum type;
	int x, y, n;
} textures[TEXTURE_COUNT];

#endif // GLOBAL_H
