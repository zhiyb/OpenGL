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
	float height(float x, float z);
	void addVertex(float x, float z);
	void addVertex(glm::vec3 vertex);
};

#endif // GROUND_H
