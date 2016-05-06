#ifndef SQUARE_H
#define SQUARE_H

#include "object.h"

class Square : public Object
{
public:
	Square() {setup();}
	void render();

protected:
	void setup();
};

#endif // SQUARE_H
