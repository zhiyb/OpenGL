#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include "global.h"
#include "object.h"

using namespace glm;

Object::Object()
{

}

void Object::setup()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint bVertex;
	glGenBuffers(1, &bVertex);
	glBindBuffer(GL_ARRAY_BUFFER, bVertex);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location["position"]);
	glVertexAttribPointer(location["position"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint bNormal;
	glGenBuffers(1, &bNormal);
	glBindBuffer(GL_ARRAY_BUFFER, bNormal);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location["normal"]);
	glVertexAttribPointer(location["normal"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint bIndex;
	glGenBuffers(1, &bIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void Object::bind()
{
	glBindVertexArray(vao);
}
