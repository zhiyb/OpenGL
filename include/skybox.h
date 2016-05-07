#ifndef SKYBOX_H
#define SKYBOX_H

#include "object.h"

class Skybox : public Object
{
public:
	Skybox() {setup();}
	//void renderWireframe();
	void render();
	//btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

protected:
	void setup();

private:
	void setupVertices();
};

#endif // SKYBOX_H
