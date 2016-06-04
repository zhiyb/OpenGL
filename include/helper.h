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

// Cartesian coordinates to Spherical coordinates conversion
static inline glm::vec3 spherical(glm::vec3 v)
{
	return glm::vec3(glm::length(v),
			 acos(glm::dot(glm::normalize(v), glm::vec3(0.f, 1.f, 0.f))),
			 atan2(v.z, v.x));
}

// Spherical coordinates to Cartesian coordinates conversion
static inline glm::vec3 cartesian(glm::vec3 v)
{
	return v.x * glm::vec3(sin(v.y) * cos(v.z),
			  cos(v.y),
			  sin(v.y) * sin(v.z));
}

static inline glm::vec3 polarMix(glm::vec3 p1, glm::vec3 p2, glm::vec3 c, float r)
{
	p1 = spherical(p1 - c);
	p2 = spherical(p2 - c);
	float dphi = p2.y - p1.y;
	float dtheta = p2.z - p1.z;
	if (dphi >= PI)
		dphi -= 2.f * PI;
	else if (dphi < -PI)
		dphi += 2.f * PI;
	if (dtheta >= PI)
		dtheta -= 2.f * PI;
	else if (dtheta < -PI)
		dtheta += 2.f * PI;
	p2.y = p1.y + dphi;
	p2.z = p1.z + dtheta;
	return glm::vec3(c + cartesian(glm::mix(p1, p2, r)));
}

#endif // HELPER_H
