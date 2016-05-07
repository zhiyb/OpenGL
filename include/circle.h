#ifndef CIRCLE_H
#define CIRCLE_H

#include "object.h"

// Circle at x-z plane, normal at y+ direction
class Circle : public Object
{
public:
	Circle(GLuint steps = 12) {setup(steps);}
	void render();

protected:
	void setup(GLuint steps = 12);

private:
	void setupVertices(GLuint steps = 12);
	void addVertex(glm::vec3 vertex);
};

#endif // CIRCLE_H
