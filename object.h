#ifndef OBJECT_H
#define OBJECT_H

class Object
{
public:
	Object();
	virtual void setup();
	virtual void bind();
	virtual void renderSolid() {}
	virtual void renderWireframe() {}
	virtual void renderPoints() {}
	virtual void renderNormal() {}

protected:
	GLuint vao;
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> texCoords;
	std::vector<GLuint> indices;
};

#endif // OBJECT_H
