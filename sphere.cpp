#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include "global.h"
#include "sphere.h"

using namespace std;
using namespace glm;

void Sphere::setup()
{
	setupVertices();
	setupSolidIndices();
	setupFrameIndices();
	setupVAO();
}

void Sphere::render()
{
	glDrawElements(GL_TRIANGLE_STRIP, offset.count.solid, GL_UNSIGNED_INT, (void *)(offset.index.solid * sizeof(GLint)));
}

void Sphere::renderWireframe()
{
	glDrawElements(GL_LINES, offset.count.frame, GL_UNSIGNED_INT, (void *)(offset.index.frame * sizeof(GLint)));
}

void Sphere::renderPoints()
{
	glDrawArrays(GL_POINTS, 0, offset.count.vertex);
}

void Sphere::renderNormal()
{
	glDrawElements(GL_LINES, offset.count.normalView, GL_UNSIGNED_INT, (void *)(offset.index.normalView * sizeof(GLuint)));
}

#ifdef BULLET
btRigidBody *Sphere::createRigidBody(btScalar mass, btScalar scale, btTransform t)
{
	btCollisionShape* fallshape = new btSphereShape(scale);
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(t);
	btVector3 fallInertia(0,0,0);
	fallshape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallshape, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	//fallRigidBody->setLinearVelocity(btVector3(-10, 20, 0));
	//fallRigidBody->setRestitution(COE);
	//dynamicsWorld->addRigidBody(fallRigidBody);
	//fallRigidBody->getCollisionShape()->setLocalScaling(btVector3(0.5f, 0.5f, 0.5f));
	return fallRigidBody;
}
#endif

void Sphere::addVertex(const vec3 &vertex)
{
	vertices.push_back(vertex);
	normals.push_back(glm::normalize(vertex));
}

void Sphere::setupVertices()
{
	for (unsigned int j = 0; j <= steps; j++) {
		GLfloat theta = PI * (GLfloat)j / (GLfloat)steps;
		GLfloat z = cos(theta);
		for (unsigned int i = 0; i <= steps; i++) {
			GLfloat phi = PI * 2.f * (GLfloat)i / (GLfloat)steps;
			GLfloat x = sin(theta) * cos(phi);
			GLfloat y = sin(theta) * sin(phi);
			addVertex(vec3(x, y, z));
			texCoords.push_back(vec2((GLfloat)i / steps, (GLfloat)j / steps));
		}
	}
	offset.count.vertex = vertices.size();

	offset.normalView = vertices.size();
	for (unsigned int i = 0; i < offset.normalView; i++)
		addVertex(vertices[i] + 0.2f * normals[i]);
	offset.index.normalView = indices.size();
	for (unsigned int i = 0; i < offset.count.vertex; i++) {
		indices.push_back(i);
		indices.push_back(i + offset.normalView);
	}
	offset.count.normalView = indices.size() - offset.index.normalView;
}

void Sphere::setupSolidIndices()
{
	offset.index.solid = indices.size();
	GLushort index = 0;
	for (GLushort j = 0; j < steps; j++) {
		for (GLushort i = 0; i <= steps; i++) {
			indices.push_back(index + i);
			indices.push_back(index + i + steps + 1);
		}
		index += steps + 1;
	}
	offset.count.solid = indices.size() - offset.index.solid;
}

void Sphere::setupFrameIndices()
{
	offset.index.frame = indices.size();
	unsigned int index = 0;
	for (unsigned int j = 0; j < steps; j++) {
		if (j != 0)
			for (unsigned int i = 0; i < steps; i++) {
				indices.push_back(index + i);
				indices.push_back(index + i + 1);
			}
		for (unsigned int i = 0; i < steps; i++) {
			indices.push_back(index + i);
			indices.push_back(index + i + steps + 1);
		}
		index += steps + 1;
	}
	offset.count.frame = indices.size() - offset.index.frame;
}
