#ifndef CUBE_H
#define CUBE_H

#include "object.h"

class Cube : public Object
{
public:
	Cube() {setup();}
	void renderWireframe();
	void render();
#ifdef BULLET
	btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);
#endif

protected:
	void setup();

private:
	void setupVertices();
};

#endif // CUBE_H
