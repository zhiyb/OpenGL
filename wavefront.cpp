#include <iostream>
#include <sstream>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <stb_image.h>
#include "world.h"
#include "global.h"
#include "camera.h"
#include "helper.h"
#include "bullet.h"
#include "wavefront.h"
#include "cube.h"

#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

//#define WAVEFRONT_DEBUG

using namespace std;
using namespace glm;
using namespace tinyobj;

void Wavefront::useMaterial(const int i)
{
	if (i == materialID)
		return;
	const material_t &material = materials.at(i);

	if (material.diffuse_texname.empty())
		glBindTexture(GL_TEXTURE_2D, ::textures[TEXTURE_WHITE].id);
	else
		glBindTexture(GL_TEXTURE_2D, textures[material.diffuse_texname]);

	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;
	glUniform3fv(uniforms[UNIFORM_ENVIRONMENT], 1, (GLfloat *)&environment.ambient);
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, material.ambient);
	glUniform3fv(uniforms[UNIFORM_DIFFUSE], 1, material.diffuse);
	glUniform3fv(uniforms[UNIFORM_EMISSION], 1, material.emission);
	glUniform3fv(uniforms[UNIFORM_SPECULAR], 1, material.specular);
	glUniform1f(uniforms[UNIFORM_SHININESS], material.shininess);
}

void Wavefront::render()
{
	if (!loaded)
		return;
	materialID = -1;
	programID = 0;

	unsigned int shapeID = 0;
	for (const shape_t &shape: shapes) {
		glBindVertexArray(vaos[shapeID++]);
		//checkError("Wavefront: binding vertex array");
		const mesh_t &mesh = shape.mesh;
		if (status.shadow) {
			glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
			continue;
		}

		if (mesh.indices.size() < 3)
			continue;

		unsigned int start = 0;
		int mtl = mesh.material_ids.at(mesh.indices.at(0) / 3);
		for (unsigned int i = start + 3; i + 2 < mesh.indices.size(); i += 3) {
			int mtl2 = mesh.material_ids.at(mesh.indices.at(i) / 3);
			if (mtl == mtl2)
				continue;
			useMaterial(mtl);
			mtl = mtl2;
			glDrawElements(GL_TRIANGLES, i - start, GL_UNSIGNED_INT, (void *)(start * sizeof(GLuint)));
			//checkError("Wavefront: draw elements");
			start = i;
		}
		useMaterial(mtl);
		glDrawElements(GL_TRIANGLES, mesh.indices.size() - start, GL_UNSIGNED_INT, (void *)(start * sizeof(GLuint)));
		//checkError("Wavefront: draw elements");
	}
}

btRigidBody *Wavefront::createRigidBody(const btVector3 &scale, const float mass)
{
	btTransform t = btTransform(btQuaternion(0, 0, 0, 1),
				    scale * to_btVector3(boundingBox.origin));
	btCollisionShape* fallshape = new btBoxShape(to_btVector3(boundingBox.size / 2.f) * scale);
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(t);
	btVector3 fallInertia(0,0,0);
	fallshape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallshape, fallInertia);
	btRigidBody* rigidBody = new btRigidBody(fallRigidBodyCI);
	return rigidBody;
}

void Wavefront::createStaticRigidBody(vector<btRigidBody *> *rigidBodies, const btVector3 &scale)
{
	for (const shape_t &shape: shapes) {
		const mesh_t &mesh = shape.mesh;
		btTriangleIndexVertexArray *btMesh = new btTriangleIndexVertexArray(
					mesh.indices.size() / 3,
					(int *)mesh.indices.data(), 3 * sizeof(GLuint),
					mesh.positions.size(),
					(btScalar *)mesh.positions.data(), 3 * sizeof(float));
		btBvhTriangleMeshShape *btShape = new btBvhTriangleMeshShape(btMesh, true);
		btShape->setLocalScaling(scale);
		btDefaultMotionState* motionState = new btDefaultMotionState;
		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, btShape);
		rigidBodies->push_back(new btRigidBody(rigidBodyCI));
	}
}

void Wavefront::setup(const char *file, const char *mtlDir, const char *texDir)
{
	loaded = false;
	std::string err;
	if (!LoadObj(shapes, materials, err, file, mtlDir)) {
		cerr << "Unable to load wavefront file " << file << ":" << endl;
		cerr << err << endl;
		return;
	}
#ifdef WAVEFRONT_DEBUG
	debugPrint();
#endif
	this->texDir = texDir;
	for (material_t &material: materials) {
		if (!material.diffuse_texname.empty()) {
			string &texname = material.diffuse_texname;
			texname = basename(texname);
			if (textures.find(texname) == textures.end())
				textures[texname] = loadTexture((texDir + texname).c_str()).id;
		}
	}

	GLuint vaos[shapes.size()];
	glGenVertexArrays(shapes.size(), vaos);
	checkError("Wavefront: generating vertex array objects");
	this->vaos = std::vector<GLuint>(vaos, vaos + shapes.size());

	unsigned int i = 0;
	for (const shape_t &shape: shapes) {
		//clog << "Generating VAO for shape " << shape.name << endl;
		const mesh_t &mesh = shape.mesh;
		glBindVertexArray(vaos[i]);

		GLuint buffers[4];
		glGenBuffers(4, buffers);

		if (!mesh.positions.empty()) {
			//clog << "\tstep: positions" << endl;
			glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
			glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(float), mesh.positions.data(), GL_STATIC_DRAW);
			if (checkError("Wavefront: uploading vertices"))
				return;
			glEnableVertexAttribArray(ATTRIB_POSITION);
			glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (!mesh.normals.empty()) {
			//clog << "\tstep: normals" << endl;
			glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
			glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(float), mesh.normals.data(), GL_STATIC_DRAW);
			if (checkError("Wavefront: uploading normals"))
				return;
			glEnableVertexAttribArray(ATTRIB_NORMAL);
			glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (!mesh.texcoords.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
			glBufferData(GL_ARRAY_BUFFER, mesh.texcoords.size() * sizeof(float), mesh.texcoords.data(), GL_STATIC_DRAW);
			if (checkError("Wavefront: uploading texture coordinates"))
				return;
			glEnableVertexAttribArray(ATTRIB_TEXCOORD);
			glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (!mesh.indices.empty()) {
			//clog << "\tstep: indices" << endl;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
			if (checkError("Wavefront: uploading indices"))
				return;
		}
		i++;
	}
	calcBoundingBox();
	loaded = true;
}

void Wavefront::calcBoundingBox()
{
	vec3 min, max;
	bool first = true;
	for (const shape_t &shape: shapes) {
		const std::vector<float> &positions = shape.mesh.positions;
		for (unsigned int i = 0; i < positions.size(); i += 3) {
			vec3 pos(positions[i], positions[i + 1], positions[i + 2]);
			if (first) {
				first = false;
				max = min = pos;
			} else {
				max = glm::max(max, pos);
				min = glm::min(min, pos);
			}
		}
	}
	boundingBox.size = max - min;
	boundingBox.origin = (max + min) / 2.f;
	//boundingBox.debugPrint();
}

void Wavefront::debugPrint() const
{
	clog << __PRETTY_FUNCTION__ << endl;
	int i = 0;
	for (const shape_t &shape: shapes) {
		clog << "S/" << i++ << ": " << shape.name << endl;
#if 0
		for (const unsigned char nvec: shape.mesh.num_vertices) {
			if (nvec != 3)
				clog << (unsigned int)nvec << ", ";
		}
		clog << endl;
#endif
		clog << "\tsizeof positions: " << shape.mesh.positions.size() << endl;
		clog << "\tsizeof normals: " << shape.mesh.normals.size() << endl;
		clog << "\tsizeof texcoords: " << shape.mesh.texcoords.size() << endl;
		clog << "\tsizeof indices: " << shape.mesh.indices.size() << endl;
		clog << "\tsizeof num_vertices: " << shape.mesh.num_vertices.size() << endl;
		clog << "\tsizeof material_ids: " << shape.mesh.material_ids.size() << endl;
		clog << "\tsizeof tags: " << shape.mesh.tags.size() << endl;
	}
	i = 0;
	for (const material_t &material: materials) {
		clog << "M/" << i++ << ": " << material.name << endl;
		if (!material.ambient_texname.empty())
			clog << "\tmap_Ka: " << material.ambient_texname << endl;
		if (!material.diffuse_texname.empty())
			clog << "\tmap_Kd: " << material.diffuse_texname << endl;
		if (!material.specular_texname.empty())
			clog << "\tmap_Ks: " << material.specular_texname << endl;
		if (!material.specular_highlight_texname.empty())
			clog << "\tmap_Ns: " << material.specular_highlight_texname << endl;
		if (!material.bump_texname.empty())
			clog << "\tmap_bump: " << material.bump_texname << endl;
		if (!material.displacement_texname.empty())
			clog << "\tmap_disp: " << material.displacement_texname << endl;
		if (!material.alpha_texname.empty())
			clog << "\tmap_d: " << material.alpha_texname << endl;
		clog << "\tambient: " << material.ambient[0] << ", " << material.ambient[1] << ", " << material.ambient[2] << endl;
		clog << "\tdiffuse: " << material.diffuse[0] << ", " << material.diffuse[1] << ", " << material.diffuse[2] << endl;
		clog << "\tspecular: " << material.specular[0] << ", " << material.specular[1] << ", " << material.specular[2] << endl;
		clog << "\ttransmittance: " << material.transmittance[0] << ", " << material.transmittance[1] << ", " << material.transmittance[2] << endl;
		clog << "\temission: " << material.emission[0] << ", " << material.emission[1] << ", " << material.emission[2] << endl;
		clog << "\tshininess: " << material.shininess << endl;
		clog << "\tior: " << material.ior << endl;
		clog << "\tdissolve: " << material.dissolve << endl;
		clog << "\tillum: " << material.illum << endl;
		//clog << endl;

		// illum parameter: (http://www.fileformat.info/format/material/)
		// 0 Color on and Ambient off
		// 1 Color on and Ambient on
		// 2 Highlight on
		// 3 Reflection on and Ray trace on
		// 4 Transparency: Glass on
		// 	Reflection: Ray trace on
		// 5 Reflection: Fresnel on and Ray trace on
		// 6 Transparency: Refraction on
		// 	Reflection: Fresnel off and Ray trace on
		// 7 Transparency: Refraction on
		// 	Reflection: Fresnel on and Ray trace on
		// 8 Reflection on and Ray trace off
		// 9 Transparency: Glass on
		// 	Reflection: Ray trace off
		// 10 Casts shadows onto invisible surfaces
	}
}

void Wavefront::BoundingBox::debugPrint() const
{
	clog << __PRETTY_FUNCTION__ << ": origin = " << origin << ", size = " << size << endl;
}
