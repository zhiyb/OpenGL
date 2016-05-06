#include "ground.h"

#define SCALE	15.f

using namespace glm;

void Ground::render()
{
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
}

void Ground::setup()
{
	addVertex(vec3(0.f));
	const unsigned int steps = 12;
	for (unsigned int i = 0; i <= steps; i++) {
		float theta = 2.f * PI * i / steps;
		addVertex(vec3(cos(theta), 0.f, -sin(theta)) * SCALE);
	}
	setupVAO();
}

void Ground::addVertex(glm::vec3 vertex)
{
	vertices.push_back(vertex);
	normals.push_back(vec3(0.f, 1.f, 0.f));
	texCoords.push_back(vec2(vertex.x, vertex.z));
}
