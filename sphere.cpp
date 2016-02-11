#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include "global.h"
#include "sphere.h"

#define TRIANGLES	4

using namespace std;
using namespace glm;

void Sphere::setup()
{
	setupVertices();
	setupSolidIndices();
	setupFrameIndices();
	Object::setup();
}

void Sphere::renderSolid()
{
	GLuint index = offset.index.solid * sizeof(GLuint);
	glDrawElements(GL_TRIANGLE_FAN, 2 + steps, GL_UNSIGNED_INT, (void *)index);
	index += sizeof(GLuint) * (steps + 2);
	glDrawElements(GL_TRIANGLE_STRIP, (steps - 2) * (2 + steps * 2), GL_UNSIGNED_INT, (void *)index);
	index += sizeof(GLuint) * (steps - 2) * (2 + steps * 2);
	glDrawElements(GL_TRIANGLE_FAN, 2 + steps, GL_UNSIGNED_INT, (void *)index);
}

void Sphere::renderFrame()
{
	glDrawElements(GL_LINES, offset.count.frame, GL_UNSIGNED_INT, (void *)(offset.index.frame * sizeof(GLint)));
}

void Sphere::renderPoints()
{
	glDrawArrays(GL_POINTS, offset.vertex, offset.count.vertex);
}

void Sphere::renderNormal()
{
	glDrawElements(GL_LINES, offset.count.normalView, GL_UNSIGNED_INT, (void *)(offset.index.normalView * sizeof(GLuint)));
}

void Sphere::addVertex(const glm::vec3 &vertex)
{
	vertices.push_back(vertex);
	normals.push_back(glm::normalize(vertex));
}

void Sphere::setupVertices()
{
	offset.vertex = vertices.size();
	addVertex(glm::vec3(0.f, 0.f, 1.f));
	for (unsigned int j = 1; j < steps; j++) {
		GLfloat theta = glm::pi<GLfloat>() * (GLfloat)j / (GLfloat)steps;
		GLfloat z = cos(theta);
		for (unsigned int i = 0; i < steps; i++) {
			GLfloat phi = glm::pi<GLfloat>() * 2.f * (GLfloat)i / (GLfloat)steps;
			GLfloat x = sin(theta) * cos(phi);
			GLfloat y = sin(theta) * sin(phi);
			addVertex(glm::vec3(x, y, z));
		}
	}
	addVertex(glm::vec3(0.f, 0.f, -1.f));
	offset.count.vertex = vertices.size() - offset.vertex;

	offset.normalView = vertices.size();
	for (unsigned int i = offset.vertex; i < offset.normalView; i++)
		addVertex(vertices[i] + 0.2f * normals[i]);
	offset.index.normalView = indices.size();
	for (unsigned int i = 0; i < offset.count.vertex; i++) {
		indices.push_back(i + offset.vertex);
		indices.push_back(i + offset.normalView);
	}
	offset.count.normalView = indices.size() - offset.index.normalView;
}

void Sphere::setupSolidIndices()
{
	offset.index.solid = indices.size();
	GLushort index = offset.vertex + 1;
	indices.push_back(offset.vertex);
	for (GLushort i = 0; i < steps; i++)
		indices.push_back(index + i);
	indices.push_back(index);
	for (GLushort j = 0; j < steps - 2; j++) {
		for (GLushort i = 0; i < steps; i++) {
			indices.push_back(index + i);
			indices.push_back(index + i + steps);
		}
		indices.push_back(index);
		indices.push_back(index + steps);
		index += steps;
	}
	// For bottom, the order need to be inverted, for normal direction
	indices.push_back(index + steps);
	for (GLushort i = 0; i < steps; i++)
		indices.push_back(index + steps - 1 - i);
	indices.push_back(index + steps - 1);
	offset.count.solid = indices.size() - offset.index.solid;
}

void Sphere::setupFrameIndices()
{
	offset.index.frame = indices.size();
	unsigned int index = offset.vertex + 1;
	for (unsigned int i = 0; i < steps; i++) {
		indices.push_back(offset.vertex);
		indices.push_back(index + i);
	}
	for (unsigned int j = 0; j < steps - 1; j++) {
		for (unsigned int i = 0; i < steps; i++) {
			indices.push_back(index + i);
			indices.push_back(i == steps - 1 ? index : index + i + 1);
		}
		if (j == steps - 2)
			break;
		for (unsigned int i = 0; i < steps; i++) {
			indices.push_back(index + i);
			indices.push_back(index + i + steps);
		}
		index += steps;
	}
	for (unsigned int i = 0; i < steps; i++) {
		indices.push_back(index + i);
		indices.push_back(index + steps);
	}
	offset.count.frame = indices.size() - offset.index.frame;
}
