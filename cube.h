#ifndef CUBE_H
#define CUBE_H

#include "object.h"

class Cube : public Object
{
public:
	Cube();
	void setup();
	void renderWireframe();
	void renderSolid();
	btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

private:
	void setupVertices();
};

#endif // CUBE_H
