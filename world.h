#ifndef WORLD_H
#define WORLD_H

#include "global.h"

#define GRAVITY		-9.81f
#define RESTITUTION	1.f
#define FRICTION	0.1f

#define AMBIENT		0.8f

extern struct environment_t {
	environment_t() : ambient(AMBIENT, AMBIENT, AMBIENT) {}
	glm::vec3 ambient;
} environment;

#endif // WORLD_H
