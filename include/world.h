#ifndef WORLD_H
#define WORLD_H

#include "global.h"

extern class status_t
{
public:
	void pause(bool e);

	bool run;
	double animation, pauseStart, pauseDuration;
	enum {CameraMode, TourMode} mode;
} status;

extern class matrix_t
{
public:
	void update();

	glm::mat4 model, view, projection;
	glm::mat4 mvp;
	glm::mat3 normal;
} matrix;

extern class environment_t
{
public:
	environment_t();
	~environment_t();

	enum DaylightStatus {Sunrise, Daytime, Sunset, Night};

	void load();
	void update(float time);
	void setup();
	void print();
	void render();
	enum DaylightStatus status() {return day.status;}

	glm::vec3 ambient;
	struct {
		glm::vec3 direction, intensity;
	} light;

private:
	struct {
		enum DaylightStatus status;
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
		glm::vec3 initial, axis, colour, moon;
		float size;
	} sun;

	struct {
		class Skybox *skybox;
		class Circle *sun;
		class Ground *ground;
	} mesh;

	double time;
	class btRigidBody *groundRigidBody;
} environment;

#endif // WORLD_H