#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include "global.h"

struct shader_t {
	GLenum type;
	const char *path;
};

struct model_t {
	model_t() : model(0) {}
	class Wavefront *model;
	std::string id;
	glm::vec3 scale;
	float mass;
	bool culling;
	bool bullet, upright;
};
extern std::unordered_map<std::string, model_t> models;

struct object_t {
	std::string id;
	glm::vec3 pos;
	model_t *model;
	btRigidBody *rigidBody;
};
extern std::unordered_map<std::string, object_t> objects;

struct light_t {
	bool enabled;
	glm::vec3 ambient;
	glm::vec3 colour;
	glm::vec3 position;
	GLint shadow;
	float attenuation;
};
extern std::unordered_map<std::string, light_t> lights;

std::string basename(std::string &path);
static inline std::istream &operator>>(std::istream &stream, glm::vec3 &vec)
{return stream >> vec.x >> vec.y >> vec.z;}
static inline std::istream &operator>>(std::istream &stream, glm::vec4 &vec)
{return stream >> vec.x >> vec.y >> vec.z >> vec.w;}
static inline std::istream &operator>>(std::istream &stream, glm::quat &q)
{return stream >> q.x >> q.y >> q.z >> q.w;}

static inline std::ostream &operator<<(std::ostream &stream, const glm::vec3 &vec)
{return stream << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';}
static inline std::ostream &operator<<(std::ostream &stream, const glm::quat &q)
{return stream << '(' << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ')';}

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
void setLights(uniformMap &uniforms);

void loadModels();
void cleanupModels();
void loadObjects();

#endif // HELPER_H
