#ifndef CUBEENCLOSED_H
#define CUBEENCLOSED_H

#include "object.h"

class CubeEnclosed : public Object
{
public:
	CubeEnclosed() {setup();}
	void renderWireframe();
	void render();
#if 0
	btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);
#endif

protected:
	void setup();

private:
	void setupVertices();
};

#endif // CUBEENCLOSED_H
