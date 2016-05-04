#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include "global.h"
#include "skybox.h"

using namespace glm;

void Skybox::setup()
{
	setupVertices();
	setupVAO();
}

void Skybox::render()
{
	for (int i = 0; i < 6; i++)
		glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
}

void Skybox::setupVertices()
{
	const vec3 vertices[6][4] = {
		// Front
		{vec3(1.f, -1.f, 1.f), vec3(-1.f, -1.f, 1.f), vec3(-1.f, 1.f, 1.f), vec3(1.f, 1.f, 1.f),},
		// Up
		{vec3(-1.f, 1.f, -1.f), vec3(1.f, 1.f, -1.f), vec3(1.f, 1.f, 1.f), vec3(-1.f, 1.f, 1.f),},
		// Right
		{vec3(1.f, -1.f, -1.f), vec3(1.f, -1.f, 1.f), vec3(1.f, 1.f, 1.f), vec3(1.f, 1.f, -1.f),},
		// Back
		{vec3(-1.f, -1.f, -1.f), vec3(1.f, -1.f, -1.f), vec3(1.f, 1.f, -1.f), vec3(-1.f, 1.f, -1.f),},
		// Down
		{vec3(-1.f, -1.f, 1.f), vec3(1.f, -1.f, 1.f), vec3(1.f, -1.f, -1.f), vec3(-1.f, -1.f, -1.f),},
		// Left
		{vec3(-1.f, -1.f, 1.f), vec3(-1.f, -1.f, -1.f), vec3(-1.f, 1.f, -1.f), vec3(-1.f, 1.f, 1.f),},
	};
	const vec3 normals[6] = {
		vec3(0.f, 0.f, -1.f),
		vec3(0.f, -1.f, 0.f),
		vec3(-1.f, 0.f, 0.f),
		vec3(0.f, 0.f, 1.f),
		vec3(0.f, 1.f, 0.f),
		vec3(1.f, 0.f, 0.f),
	};
	const vec2 offsets[6] = {
		vec2(3.f, 1.f), vec2(1.f, 0.f), vec2(2.f, 1.f), vec2(1.f, 1.f), vec2(1.f, 2.f), vec2(0.f, 1.f),
	};
	const float l = 1.f / 256.f, h = 1.f - l;
	const vec2 texCoords[4] = {
		//vec2(0.f, 1.f), vec2(1.f, 1.f), vec2(1.f, 0.f), vec2(0.f, 0.f),
		vec2(l, h), vec2(h, h), vec2(h, l), vec2(l, l),
	};
	const vec2 region(1.f / 4.f, 1.f / 3.f);
	for (int i = 0; i != 6; i++) {
		for (int j = 0; j != 4; j++) {
			this->vertices.push_back(vertices[i][j]);
			this->normals.push_back(normals[i]);
			this->texCoords.push_back((texCoords[j] + offsets[i]) * region);
			//this->texCoords.push_back(texCoords[j]);
		}
	}
}
