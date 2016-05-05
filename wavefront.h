#ifndef WAVEFRONT_H
#define WAVEFRONT_H

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
	//void renderWireframe();
	void render();
	//btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

protected:
	void setup(const char *file, const char *mtlDir, const char *texDir);

private:
	void useMaterial(const int i);
	void debugPrint();

	bool loaded;
	int materialID;
	std::string texDir;
	std::vector<GLuint> vaos;
	std::map<std::string, GLuint> textures;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
};

#endif // WAVEFRONT_H
