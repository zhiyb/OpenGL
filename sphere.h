#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"

class Sphere : public Object
{
public:
	Sphere(GLuint steps = 12) : steps(steps) {}
	void setup();
	void renderSolid();
	void renderFrame();
	void renderPoints();
	void renderNormal();

private:
	void addVertex(const glm::vec3 &vertex);
	void setupVertices();
	void setupSolidIndices();
	void setupFrameIndices();

	GLuint steps;
	struct {
		GLuint vertex;
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
