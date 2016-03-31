#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <string>
#include "object.h"
#include "tiny_obj_loader.h"

#define WAVEFRONT_DEBUG

class Wavefront : public Object
{
public:
	Wavefront(const char *file) {setup(file);}
	void bind() {}	// It needs multiple VAOs
	//void renderWireframe();
	void renderSolid();
	//btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

protected:
	void setup(const char *file);

private:
#ifdef WAVEFRONT_DEBUG
	void debugPrint();
#endif

	std::vector<GLuint> vaos;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
};

#endif // WAVEFRONT_H
