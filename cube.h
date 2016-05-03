#ifndef CUBE_H
#define CUBE_H

#include "object.h"

class Cube : public Object
{
public:
	Cube() {setup();}
	void renderWireframe();
	void render();
	btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

protected:
	void setup();

private:
	void setupVertices();
};

#endif // CUBE_H
