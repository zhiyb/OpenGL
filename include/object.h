#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "global.h"
#ifdef BULLET
#include <btBulletDynamicsCommon.h>
#endif

class Object
{
public:
	Object() {}
	virtual ~Object() {}
	virtual void bind();
	virtual void render() {}
	virtual void renderWireframe() {}
	virtual void renderPoints() {}
	virtual void renderNormal() {}
#ifdef BULLET
	virtual btRigidBody *createRigidBody(btScalar /*mass*/, btScalar /*scale*/, btTransform /*T*/) {return 0;}
#endif

protected:
	virtual void setupVAO();

	GLuint vao;
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> texCoords;
	std::vector<GLuint> indices;
};

#endif // OBJECT_H
