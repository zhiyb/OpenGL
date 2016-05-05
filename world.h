#ifndef WORLD_H
#define WORLD_H

#include "global.h"

#define GRAVITY		-9.81f
#define RESTITUTION	1.f
#define FRICTION	0.1f

extern struct environment_t {
	environment_t()
	{
		using namespace glm;
		ambient = vec3(1.f, 0.95f, 0.95f) / 2.f;
		light.direction = vec3(0.f, 1.f, 0.f);
		light.intensity = vec3(1.f, 0.95f, 0.95f);
	}

	glm::vec3 ambient;
	struct {
		glm::vec3 direction, intensity;
	} light;
} environment;

#endif // WORLD_H
