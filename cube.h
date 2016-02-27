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

private:
	void setupVertices();
};

#endif // CUBE_H
