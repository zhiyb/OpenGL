#include <iostream>
#include <glm/gtc/noise.hpp>
#include "ground.h"
#include "helper.h"

#define SCALE	20.f

using namespace std;
using namespace glm;

void Ground::render()
{
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

btRigidBody *Ground::createRigidBody()
{
	btTriangleIndexVertexArray *btMesh = new btTriangleIndexVertexArray(
				indices.size() / 3, (int *)indices.data(), 3 * sizeof(GLuint),
				vertices.size() * 3, (btScalar *)vertices.data(), sizeof(vec3));
	btBvhTriangleMeshShape *btShape = new btBvhTriangleMeshShape(btMesh, true);
	btDefaultMotionState* motionState = new btDefaultMotionState;
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, btShape);
	return new btRigidBody(rigidBodyCI);
}

void Ground::setup()
{
	const int size = 30;
	int index = 0;
	for (int x = -size; x < size; x++)
		for (int z = -size; z < size; z++) {
			addVertex(x, z);
			addVertex(x, z + 1);
			addVertex(x + 1, z + 1);
			addVertex(x + 1, z);
			indices.push_back(index);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index);
			indices.push_back(index + 2);
			indices.push_back(index + 3);
			index += 4;
		}
	setupVAO();
}

float Ground::height(float x, float z)
{
	float d = glm::min(glm::distance(vec2(x, z), vec2(0.f)) / 15.f, 2.f);
	return perlin(vec2(x, z) / 20.f) * d * d * d * d;
}

void Ground::addVertex(float x, float z)
{
	texCoords.push_back(vec2(x, z));
	vec3 pos(x, height(x, z), z);
	vertices.push_back(pos);

	float delta = 0.05;
	vec3 posX(pos + vec3(delta, 0.f, 0.f));
	vec3 posZ(pos + vec3(0.f, 0.f, delta));
	posX.y = height(posX.x, posX.z);
	posZ.y = height(posZ.x, posZ.z);
	vec3 normal = cross(posZ - pos, posX - pos);
	normals.push_back(normal);
}
