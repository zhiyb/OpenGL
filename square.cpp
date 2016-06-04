#include "global.h"
#include "square.h"

using namespace std;
using namespace glm;

void Square::render()
{
	glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
}

void Square::setup()
{
	const vec3 vertex[] = {
		vec3(1.f, 1.f, 0.f), vec3(1.f, -1.f, 0.f), vec3(-1.f, -1.f, 0.f), vec3(-1.f, 1.f, 0.f),
	};
	for (int i = 0; i != 4; i++) {
		vertices.push_back(vertex[i]);
		texCoords.push_back((vec2(vertex[i].x, -vertex[i].y) + 1.f) / 2.f);
	}
	setupVAO();
}
