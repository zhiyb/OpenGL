#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <vector>
#include <string>
#include <map>
#include "object.h"
#include "tiny_obj_loader.h"

class Wavefront : public Object
{
public:
	Wavefront(const char *file, const char *mtlDir, const char *texDir) {setup(file, mtlDir, texDir);}
	bool isValid() const {return loaded;}
	void bind() {}	// It needs multiple VAOs, binding during rendering
	void render();
	glm::vec3 boundingOrigin() const {return boundingBox.origin();}
	glm::vec3 boundingSize() const {return boundingBox.size();}
	btRigidBody *createRigidBody(const btVector3 &scale, const float mass);
	void createStaticRigidBody(std::vector<btRigidBody *> *rigidBodies, const btVector3 &scale);

protected:
	void setup(const char *file, const char *mtlDir, const char *texDir);

private:
	void calcBoundingBox();
	void useMaterial(const int i);
	void debugPrint() const;

	bool loaded;
	int materialID;
	GLuint programID;
	std::string texDir;
	std::vector<GLuint> vaos;
	std::map<std::string, GLuint> textures;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	class BoundingBox {
	public:
		glm::vec3 size() const {return max - min;}
		glm::vec3 origin() const {return (max + min) / 2.f;}
		void debugPrint() const;

		glm::vec3 max, min;
	} boundingBox;
};

#endif // WAVEFRONT_H
