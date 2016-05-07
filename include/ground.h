#ifndef GROUND_H
#define GROUND_H

#include "object.h"

class Ground : public Object
{
public:
	Ground() {setup();}
	void render();
	btRigidBody *createRigidBody();

private:
	void setup();
	void addVertex(glm::vec3 vertex);
};

#endif // GROUND_H
