#ifndef GROUND_H
#define GROUND_H

#include "square.h"

class Ground : public Object
{
public:
	Ground() {setup();}
	void render();

private:
	void setup();
	void addVertex(glm::vec3 vertex);
};

#endif // GROUND_H
