#ifndef WAVEFRONT_H
#define WAVEFRONT_H

#include <string>
#include <map>
#include "object.h"
#include "tiny_obj_loader.h"

#define WAVEFRONT_DEBUG

class Wavefront : public Object
{
public:
	Wavefront(const char *file, const char *mtlDir, const char *texDir) {setup(file, mtlDir, texDir);}
	void bind() {}	// It needs multiple VAOs
	//void renderWireframe();
	void render();
	//btRigidBody *createRigidBody(btScalar mass, btScalar scale, btTransform T);

protected:
	void setup(const char *file, const char *mtlDir, const char *texDir);

private:
	void useMaterial(const int i);
	GLuint loadTexture(const std::string &filename);
	void basename(std::string &path);
#ifdef WAVEFRONT_DEBUG
	void debugPrint();
#endif

	int materialID;
	std::string texDir;
	std::vector<GLuint> vaos;
	std::map<std::string, GLuint> textures;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
};

#endif // WAVEFRONT_H
