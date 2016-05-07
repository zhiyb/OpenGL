#include <iostream>
#include <sstream>
#include <fstream>
#include "helper.h"
#include "camera.h"
#include "world.h"
#include "bullet.h"

#include "circle.h"
#include "skybox.h"
#include "ground.h"

using namespace std;
using namespace glm;

status_t status;
matrix_t matrix;
environment_t environment;

void status_t::pause(bool e)
{
	if (e) {
		if (!status.run)
			return;
		pauseStart = glfwGetTime();
	} else {
		if (status.run)
			return;
		pauseDuration += glfwGetTime() - pauseStart;
	}
	status.run = !status.run;
}

void matrix_t::update()
{
	mvp = projection * view * model;
	normal = mat3(transpose(inverse(model)));
}

environment_t::environment_t()
{
	ambient = vec3(1.f, 0.95f, 0.95f) / 2.f;
	light.direction = vec3(0.f, 1.f, 0.f);
	light.intensity = vec3(1.f, 0.95f, 0.95f);
	mesh.skybox = 0;
	mesh.sun = 0;
}

environment_t::~environment_t()
{
	delete mesh.skybox;
	delete mesh.sun;
}

void environment_t::load()
{
	ifstream datafs(DATA_ENVIRON);
	if (!datafs) {
		cerr << "Cannot open environment description file " DATA_ENVIRON << endl;
		return;
	}
	string line;
	while (getline(datafs, line)) {
		if (line.empty() || line.at(0) == '#')
			continue;
		istringstream ss(line);
		string param;
		ss >> param;
		if (!ss)
			continue;
		if (param == "Day") {
			ss >> day.sunrise.duration;
			ss >> day.daytime.duration;
			ss >> day.sunset.duration;
			ss >> day.night.duration;
		} else if (param == "Sunrise")
			ss >> day.sunrise.rate;
		else if (param == "Daytime")
			ss >> day.daytime.intensity >> day.daytime.ambient;
		else if (param == "Sunset")
			ss >> day.sunset.rate;
		else if (param == "Night")
			ss >> day.night.intensity >> day.night.ambient;
		else if (param == "Sun")
			ss >> sun.initial >> sun.axis >> sun.size >> sun.colour >> sun.moon;
	}
	day.duration = day.sunrise.duration + day.daytime.duration + \
			day.sunset.duration + day.night.duration;
}

void environment_t::update(float time)
{
	this->time = time;
	time = fmod(time, day.duration);

	float daytime = day.duration - day.night.duration;
	float angle = PI * time / daytime;

	if (time < day.sunrise.duration) {
		float ratio = time / day.sunrise.duration;
		ambient = day.night.ambient + min(day.sunrise.rate * ratio, vec3(1.f)) * (day.daytime.ambient - day.night.ambient);
		light.intensity = day.night.intensity + min(day.sunrise.rate * ratio, vec3(1.f)) * (day.daytime.intensity - day.night.intensity);
		day.status = Sunrise;
		goto direction;
	} else
		time -= day.sunrise.duration;

	if (time < day.daytime.duration) {
		ambient = day.daytime.ambient;
		light.intensity = day.daytime.intensity;
		day.status = Daytime;
		goto direction;
	} else
		time -= day.daytime.duration;

	if (time < day.sunset.duration) {
		float ratio = time / day.sunrise.duration;
		ambient = day.daytime.ambient - min(day.sunset.rate * ratio, vec3(1.f)) * (day.daytime.ambient - day.night.ambient);
		light.intensity = day.daytime.intensity - min(day.sunset.rate * ratio, vec3(1.f)) * (day.daytime.intensity - day.night.intensity);
		day.status = Sunset;
	} else {
		ambient = day.night.ambient;
		light.intensity = day.night.intensity;
		angle = PI * (time - day.sunset.duration) / day.night.duration;
		day.status = Night;
	}

direction:
	light.direction = normalize(vec3(rotate(quat(), angle, sun.axis) * vec4(sun.initial, 0.f)));
}

void environment_t::setup()
{
	load();
	mesh.skybox = new Skybox;
	mesh.ground = new Ground;
	mesh.sun = new Circle(32);
	bulletAddRigidBody(mesh.ground->createRigidBody(), BULLET_GROUND);
}

void environment_t::print()
{
	clog << "Environment at " << time << " seconds" << endl;
}

void environment_t::render()
{
	//glEnable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	// Render skybox
	glUseProgram(programs[PROGRAM_SKYBOX].id);
	//checkError("switching to PROGRAM_SKYBOX");
	uniformMap &uniforms = programs[PROGRAM_SKYBOX].uniforms;

	vec3 brightness = ambient + light.intensity;
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&brightness);

	matrix.model = translate(mat4(), camera.position());
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);

	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SKYBOX].texture);
	//checkError("binding TEXTURE_SKYBOX");
	mesh.skybox->bind();
	mesh.skybox->render();

	// Render sun
	if (day.status != Night)
		glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&sun.colour);
	else
		glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&sun.moon);

	matrix.model = translate(mat4(), environment.light.direction);
	matrix.model = translate(matrix.model, camera.position());
	vec3 normal = cross(vec3(0.f, -1.f, 0.f), light.direction);
	float angle = acos(dot(vec3(0.f, -1.f, 0.f), light.direction));
	matrix.model = rotate(matrix.model, angle, normal);
	matrix.model = scale(matrix.model, vec3(environment.sun.size));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);

	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GLOW].texture);
	//checkError("binding TEXTURE_GLOW");
	mesh.sun->bind();
	mesh.sun->render();

	// Render ground
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&brightness);

	vec3 pos(floor(camera.position()));
	matrix.model = translate(mat4(), vec3(pos.x, 0.f, pos.z));
	//matrix.model = scale(matrix.model, vec3(0.1f));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);

	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GROUND].texture);
	//checkError("binding TEXTURE_GLOW");
	mesh.ground->bind();

	glDepthMask(GL_TRUE);
	mesh.ground->render();

	//glClear(GL_DEPTH_BUFFER_BIT);
}
