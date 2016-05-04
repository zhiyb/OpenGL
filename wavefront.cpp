#include <iostream>
#include <sstream>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <stb_image.h>
#include "world.h"
#include "global.h"
#include "wavefront.h"

#define WAVEFRONT_DEBUG

using namespace std;
using namespace glm;
using namespace tinyobj;

void Wavefront::useMaterial(const int i)
{
	if (i == materialID)
		return;
	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	// Material properties
	const material_t &material = materials.at(i);
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&environment.ambient);
	//glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, material.ambient);
	glUniform3fv(uniforms[UNIFORM_DIFFUSE], 1, material.diffuse);
	glUniform3fv(uniforms[UNIFORM_SPECULAR], 1, material.specular);
	glUniform1f(uniforms[UNIFORM_SHININESS], material.shininess);

	if (!material.diffuse_texname.empty()) {
		glBindTexture(GL_TEXTURE_2D, textures[material.diffuse_texname]);
		glUniform1ui(uniforms[UNIFORM_TEXTURED], 1);
	} else {
		glUniform1ui(uniforms[UNIFORM_TEXTURED], 0);
		//clog << __func__ << ": No texture";
	}
}

GLuint Wavefront::loadTexture(const string &filename)
{
	string path = texDir + filename;
	texture_t tex;
	unsigned char *data = stbi_load(path.c_str(), &tex.x, &tex.y, &tex.n, 3);
	if (data == 0) {
		cerr << "Error loading texture file " << path << endl;
		return 0;
	}
	if (tex.n != 3) {
		cerr << "Invalid image format from texture file " << path << endl;
		stbi_image_free(data);
		return 0;
	}
#ifdef WAVEFRONT_DEBUG
	clog << __func__ << ": Texture file " << path << " loaded, " << tex.x << "x" << tex.y << "-" << tex.n << endl;
#endif
	glGenTextures(1, &tex.texture);
	if (tex.texture == 0) {
		cerr << "Cannot generate texture storage for " << path << endl;
		stbi_image_free(data);
		return 0;
	}
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex.x, tex.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	return tex.texture;
}

void Wavefront::basename(string &path)
{
	string token;
	{
		istringstream iss(path);
		while (getline(iss, token, '/'))
			path = token;
	}
	{
		istringstream iss(path);
		while (getline(iss, token, '\\'))
			path = token;
	}
	//clog << __func__ << ": " << path << endl << token << endl;
}

void Wavefront::render()
{
	materialID = -1;
	unsigned int shapeID = 0;
	for (const shape_t &shape: shapes) {
		glBindVertexArray(vaos[shapeID]);
#if 1
		const mesh_t &mesh = shape.mesh;
		for (unsigned int i = 0; i + 2 < mesh.indices.size(); i += 3) {
			useMaterial(mesh.material_ids.at(mesh.indices.at(i) / 3));
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)(i * sizeof(GLuint)));
		}
#else
		useMaterial(shape.mesh.material_ids.at(0));
		glDrawElements(GL_TRIANGLES, shape.mesh.indices.size(), GL_UNSIGNED_INT, 0);
#endif
		shapeID++;
	}
}

void Wavefront::setup(const char *file, const char *mtlDir, const char *texDir)
{
	std::string err;
	if (!LoadObj(shapes, materials, err, file, mtlDir)) {
		cerr << "Unable to load wavefront file " << file << ":" << endl;
		cerr << err << endl;
	}
	this->texDir = texDir;
	for (material_t &material: materials) {
		if (!material.diffuse_texname.empty()) {
			string &texname = material.diffuse_texname;
			basename(texname);
			if (textures.find(texname) == textures.end())
				textures[texname] = loadTexture(texname);
		}
	}
#ifdef WAVEFRONT_DEBUG
	debugPrint();
#endif

	GLuint vaos[shapes.size()];
	glGenVertexArrays(shapes.size(), vaos);
	this->vaos = std::vector<GLuint>(vaos, vaos + shapes.size());

	unsigned int i = 0;
	for (const shape_t &shape: shapes) {
#ifdef WAVEFRONT_DEBUG
		clog << "Generating VAO for shape " << shape.name << endl;
#endif
		const mesh_t &mesh = shape.mesh;
		glBindVertexArray(vaos[i]);

		GLuint buffers[4];
		glGenBuffers(4, buffers);

		//clog << "\tstep: positions" << endl;
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(float), mesh.positions.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_POSITION);
		glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//clog << "\tstep: normals" << endl;
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(float), mesh.normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_NORMAL);
		glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ARRAY_BUFFER, mesh.texcoords.size() * sizeof(float), mesh.texcoords.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_TEXCOORD);
		glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);

		//clog << "\tstep: indices" << endl;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
		i++;
	}
}

#ifdef WAVEFRONT_DEBUG
void Wavefront::debugPrint()
{
	clog << __PRETTY_FUNCTION__ << endl;
	for (const shape_t &shape: shapes) {
		clog << "S: " << shape.name << endl;
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

#if 0
		clog << "Indices: ";
		for (int id: shape.mesh.indices) {
			clog << id << ", ";
		}
		clog << endl;
#endif

		bool diff = false;
		int previd = -1;
		for (int id: shape.mesh.material_ids) {
			if (previd == -1)
				previd = id;
			else if (id != previd)
				diff = true;
		}

		if (diff)
			clog << "Different materials in this shape!" << endl;
	}
	for (const material_t &material: materials) {
		clog << "M: " << material.name << endl;
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
#endif
