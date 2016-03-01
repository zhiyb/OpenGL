#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>

#include <GL/glew.h>

// Different programs
enum {
	PROGRAM_BASIC = 0,
	PROGRAM_LIGHTING,
	PROGRAM_TEXTURE,
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
	UNIFORM_SPECULARPOWER,
	UNIFORM_VIEWER,
	UNIFORM_LIGHT,
	UNIFORM_COLOUR,
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
