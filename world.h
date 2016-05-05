#ifndef WORLD_H
#define WORLD_H

#include "global.h"

#define GRAVITY		-9.81f
#define RESTITUTION	1.f
#define FRICTION	0.1f

extern class matrix_t
{
public:
	void update();

	glm::mat4 model, view, projection;
	glm::mat4 mvp;
	glm::mat3 normal;

	struct {
		glm::quat rotation;
	} world;
} matrix;

extern class environment_t
{
public:
	environment_t();
	~environment_t();

	void load();
	void update(float time);
	void setup();
	void render();

	glm::vec3 ambient;
	struct {
		glm::vec3 direction, intensity;
	} light;

private:
	struct {
		struct {
			float duration;
			glm::vec3 rate;
		} sunrise;
		struct {
			float duration;
			glm::vec3 ambient, intensity;
		} daytime;
		struct {
			float duration;
			glm::vec3 rate;
		} sunset;
		struct {
			float duration;
			glm::vec3 ambient, intensity;
		} night;
		float duration;
	} day;
	struct {
		glm::vec3 initial, axis, colour;
		float size;
	} sun;

	struct {
		class Skybox *skybox;
		class Circle *sun;
	} mesh;
} environment;

#endif // WORLD_H
