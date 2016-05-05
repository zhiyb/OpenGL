#include "circle.h"

using namespace glm;

void Circle::render()
{
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
}

void Circle::setup(GLuint steps)
{
	setupVertices(steps);
	setupVAO();
}

void Circle::setupVertices(GLuint steps)
{
	addVertex(vec3(0.f));
	for (unsigned int i = 0; i <= steps; i++) {
		float theta = 2.f * PI * i / steps;
		addVertex(vec3(cos(theta), 0.f, -sin(theta)));
	}
}

void Circle::addVertex(glm::vec3 vertex)
{
	vertices.push_back(vertex);
	normals.push_back(vec3(0.f, 1.f, 0.f));
	texCoords.push_back((vec2(vertex.x, vertex.z) + 1.f) / 2.f);
}
