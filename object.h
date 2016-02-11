#ifndef OBJECT_H
#define OBJECT_H

class Object
{
public:
	Object();
	virtual void setup();
	virtual void bind();
	virtual void renderSolid() {}
	virtual void renderFrame() {}
	virtual void renderPoints() {}
	virtual void renderNormal() {}

protected:
	GLuint vao;
	std::vector<glm::vec3> vertices, normals;
	std::vector<GLuint> indices;
};

#endif // OBJECT_H
