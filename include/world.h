#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <string>
#include "global.h"

class status_t
{
public:
	status_t() : run(true), shadow(false), lines(false), animation(0.f), mode(CameraMode) {}
	void pause(bool e);

	bool run, shadow, lines;
	double animation, pauseStart, pauseDuration;
	enum {CameraMode, EnvShadowMode, TourMode} mode;
};

class matrix_t
{
public:
	void update();

	glm::mat4 model, view, projection;
	glm::mat4 mvp;
	glm::mat3 normal;
};

struct record_t {
	float time;
	struct {
		glm::vec3 pos;
		glm::quat rot;
	} camera;
};

void loadRecords();
void loadRecord(record_t &record);

struct shadow_t {
	struct env_t {
		GLuint texture;
	} environment;
	GLuint fbo;
};

class environment_t
{
public:
	environment_t();
	~environment_t();

	enum DaylightStatus {Sunrise, Daytime, Sunset, Night};

	void load();
	void update(float time);
	void setup();
	void print();
	void renderSkybox();
	void renderGround();
	enum DaylightStatus status() const {return day.status;}

	glm::vec3 ambient;
	struct {
		glm::vec3 position, intensity;
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
};

extern status_t status;
extern matrix_t matrix, shadowMatrix;
extern shadow_t shadow;
extern environment_t environment;
extern std::unordered_map<std::string, record_t> records;

#endif // WORLD_H
