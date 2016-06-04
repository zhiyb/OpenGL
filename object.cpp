#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include "global.h"
#include "object.h"

using namespace glm;

void Object::setupVAO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (!vertices.empty()) {
		GLuint bVertex;
		glGenBuffers(1, &bVertex);
		glBindBuffer(GL_ARRAY_BUFFER, bVertex);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_POSITION);
		glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if (!texCoords.empty()) {
		GLuint bTexCoord;
		glGenBuffers(1, &bTexCoord);
		glBindBuffer(GL_ARRAY_BUFFER, bTexCoord);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), texCoords.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_TEXCOORD);
		glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if (!indices.empty()) {
		GLuint bIndex;
		glGenBuffers(1, &bIndex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bIndex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	}
}

void Object::bind()
{
	glBindVertexArray(vao);
}
