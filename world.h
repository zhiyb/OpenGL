#ifndef WORLD_H
#define WORLD_H

#include "global.h"

#define GRAVITY		-9.81f
#define RESTITUTION	1.f
#define FRICTION	0.1f

#define AMBIENT		glm::vec3(1.f, 0.95f, 0.95f)

extern struct environment_t {
	environment_t() : light(AMBIENT) {}
	glm::vec3 light;
} environment;

#endif // WORLD_H
