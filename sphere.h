#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"

class Sphere : public Object
{
public:
	Sphere(GLuint steps = 12) : steps(steps) {setup();}
	void render();
	void renderWireframe();
	void renderPoints();
	void renderNormal();
#ifdef BULLET
	btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);
#endif

protected:
	void setup();

private:
	void addVertex(const glm::vec3 &vertex);
	void setupVertices();
	void setupSolidIndices();
	void setupFrameIndices();

	GLuint steps;
	struct {
		GLuint normalView;
		struct {
			GLuint solid;
			GLuint frame;
			GLuint normalView;
		} index;
		struct {
			GLuint vertex;
			GLuint solid;
			GLuint frame;
			GLuint normalView;
		} count;
	} offset;
};

#endif // SPHERE_H
