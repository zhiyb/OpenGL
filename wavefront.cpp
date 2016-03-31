#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "global.h"
#include "wavefront.h"

using namespace std;
using namespace glm;
using namespace tinyobj;

void Wavefront::renderSolid()
{
	GLint *uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	// Material properties
	GLfloat ambient[3] = {0.f, 0.f, 0.f};
	//GLfloat ambient[3] = {0.1f, 0.1f, 0.1f};
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, ambient);
	//GLfloat diffuse[3] = {1.f, 1.f, 1.f};
	//glUniform3fv(uniforms[UNIFORM_DIFFUSE], 1, diffuse);

	unsigned int i = 0;
	for (const shape_t &shape: shapes) {
		const material_t &material = materials.at(shape.mesh.material_ids.at(0));
		//glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, material.ambient);
		glUniform3fv(uniforms[UNIFORM_DIFFUSE], 1, material.diffuse);
		glUniform3fv(uniforms[UNIFORM_SPECULAR], 1, material.specular);
		glUniform1f(uniforms[UNIFORM_SHININESS], material.shininess);

		glBindVertexArray(vaos[i]);
		glDrawElements(GL_TRIANGLES, shape.mesh.indices.size(), GL_UNSIGNED_INT, 0);
		i++;
	}
}

void Wavefront::setup(const char *file)
{
	std::string err;
	if (!LoadObj(shapes, materials, err, file, "models/")) {
		cerr << "Unable to load wavefront file " << file << ":" << endl;
		cerr << err << endl;
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
		glBindVertexArray(vaos[i]);

		GLuint buffers[3];
		glGenBuffers(3, &buffers[0]);

		//clog << "\tstep: positions" << endl;
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.positions.size() * sizeof(float), shape.mesh.positions.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_POSITION);
		glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//clog << "\tstep: normals" << endl;
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, shape.mesh.normals.size() * sizeof(float), shape.mesh.normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_NORMAL);
		glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

#if 0
		GLuint bTexCoord;
		glGenBuffers(1, &bTexCoord);
		glBindBuffer(GL_ARRAY_BUFFER, bTexCoord);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), texCoords.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTRIB_TEXCOORD);
		glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
#endif

		//clog << "\tstep: indices" << endl;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.mesh.indices.size() * sizeof(unsigned int), shape.mesh.indices.data(), GL_STATIC_DRAW);
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
