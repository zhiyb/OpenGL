#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

#include "world.h"

#include "global.h"
#include "helper.h"
#include "camera.h"
#include "bullet.h"

#include "sphere.h"
#include "cube.h"
#include "skybox.h"
#include "wavefront.h"

#ifdef BULLET
#include <btBulletDynamicsCommon.h>
#endif

using namespace std;
using namespace glm;

GLFWwindow *window;

program_t programs[PROGRAM_COUNT];
texture_t textures[TEXTURE_COUNT];

struct object_t {
	bool culling, bullet;
	vec3 scale, offset;
	Object *model;
};
vector<object_t> objects;

void quit();

void setupObjects()
{
	ifstream datafs(DATA_WAVEFRONT);
	if (!datafs) {
		cerr << "Cannot open model description file " DATA_WAVEFRONT << endl;
		return;
	}
	string line;
	while (getline(datafs, line)) {
		if (line.empty() || line.at(0) == '#')
			continue;
		istringstream ss(line);
		string modelPath, mtlPath, texPath;
		object_t obj;
		ss >> modelPath >> mtlPath >> texPath;
		if (!ss)
			continue;
		ss >> obj.culling;
		ss >> obj.scale >> obj.offset >> obj.bullet;
		clog << __func__ << ": Model " << modelPath << " loading..." << endl;
		Wavefront *model = new Wavefront(modelPath.c_str(), mtlPath.c_str(), texPath.c_str());
		if (!model)
			continue;
		if (!model->isValid()) {
			delete model;
			continue;
		}
		obj.model = model;
		objects.push_back(obj);

		if (obj.bullet) {
			vector<btRigidBody *> rigidBodies;
			model->createRigidBody(&rigidBodies, to_btVector3(obj.scale));
			for (btRigidBody *rigidBody: rigidBodies) {
				rigidBody->getWorldTransform().setOrigin(to_btVector3(obj.offset));
				bulletAddRigidBody(rigidBody, BULLET_GROUND);
			}
		}
	}
}

static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	environment.render();
	camera.render();

	glUseProgram(programs[PROGRAM_WAVEFRONT].id);
	//checkError("switching to PROGRAM_WAVEFRONT");
	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	glUniform3fv(uniforms[UNIFORM_LIGHT_DIRECTION], 1, (GLfloat *)&environment.light.direction);
	glUniform3fv(uniforms[UNIFORM_LIGHT_INTENSITY], 1, (GLfloat *)&environment.light.intensity);
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&camera.position());

	for (object_t &obj: objects) {
		matrix.model = translate(mat4(), obj.offset);
		matrix.model = scale(matrix.model, obj.scale);
		matrix.update();
		glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
		glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
		glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

		if (obj.culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		obj.model->bind();
		obj.model->render();
	}
}

void step()
{
	double time = glfwGetTime();
	double diff = time - status.animation;
	status.animation = time;
	camera.updateCB(diff);

	if (status.run && diff > 0.f) {
		//clog << __func__ << ": " << diff << endl;
		time -= status.pauseDuration;
		bulletUpdate(diff);
		environment.update(time);
	}
}

void tour(const bool e)
{
}

static void refreshCB(GLFWwindow *window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//GLfloat asp = (GLfloat)height / (GLfloat)width;
	//matrix.projection = ortho<GLfloat>(-1.f, 1.f, -asp, asp, -10, 10);
	matrix.projection = perspective<GLfloat>(45.f, (GLfloat)width / (GLfloat)height, 0.01f, 1000.f);
}

static void keyCB(GLFWwindow */*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	if (key == GLFW_KEY_X) {
		camera.print();
		return;
	}

	switch (key) {
	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_Q:
		// Exit the program
		quit();
		return;
	}

	if (status.mode == status_t::TourMode) {
		if (key == GLFW_KEY_E)	// Exit the tour mode (optional)
			tour(false);
		return;
	}

	switch (key) {
	case GLFW_KEY_T:
		// Start the tour (keys except E, Q, ESC are ignored)
		tour(false);
		return;
	case GLFW_KEY_SPACE:
		// Stop all motion (optional)
		status.pause(status.run);
		return;
	case GLFW_KEY_R:
		// Reset all animation (optional)
		status.pauseDuration = glfwGetTime();
		status.run = true;
		camera.reset();
		return;
	}

	camera.keyCB(key);
}

void mouseCB(GLFWwindow */*window*/, int button, int action, int /*mods*/)
{
	camera.mouseCB(button, action);
}

void scrollCB(GLFWwindow */*window*/, double /*xoffset*/, double yoffset)
{
	camera.scrollCB(yoffset);
}

void cursorCB(GLFWwindow */*window*/, double xpos, double ypos)
{
	camera.cursorCB(xpos, ypos);
}

void quit()
{
	glfwTerminate();

	// Free memory
#ifndef MODELS
	for (object_t &obj: objects)
		delete obj.model;
#else
	for (Model *model: models) {
#ifdef BULLET
		for (btRigidBody *body: model->bodies) {
			dynamicsWorld->removeRigidBody(body);
			delete body;
		}
#endif
		delete model->object;
	}
#endif
	bulletCleanup();
	exit(0);
}

int main(int /*argc*/, char */*argv*/[])
{
	if (!glfwInit()) {
		cerr << "glfwInit failed" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
	if (!window) {
		cerr << "Cannot create glfw Window" << endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cerr << "Cannot initialise glew" << endl;
		glfwTerminate();
		return -1;
	}

	if (!GLEW_VERSION_3_3) {
		cerr << "OpenGL version 3.3 not supported by this system" << endl;
		glfwTerminate();
		return -1;
	}

#if 0	// GLFW supports cube_map, but GLEW does not?
	clog << (glIsEnabled(GL_TEXTURE_CUBE_MAP_ARB) ? "Yes" : "No") << endl;
	clog << (GLEW_ARB_texture_cube_map ? "Yes" : "No") << endl;
	clog << (glfwExtensionSupported("GL_ARB_texture_cube_map") == GL_TRUE ? "Yes" : "No") << endl;
	clog << (glewIsSupported("GL_ARB_texture_cube_map") == GL_TRUE ? "Yes" : "No") << endl;

	GLint n, i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	clog << "Extensions: " << n << endl;
	for (i = 0; i < n; i++)
		clog << glGetStringi(GL_EXTENSIONS, i) << endl;
#endif

	glGetError();	// There is an initialisation error
	//checkError("initialisation");
	glEnable(GL_DEPTH_TEST);
	checkError("enabling depth test");
#ifdef CULL_FACE
	glEnable(GL_CULL_FACE);
	checkError("enabling back face culling");
#endif
#ifdef ALPHA_BLEND
	glEnable(GL_BLEND);
	checkError("enabling alpha blending");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	glEnable(GL_TEXTURE_2D);
	glGetError();	// Ignore the error
	//checkError("enabling texture 2D");
	//glEnable(GL_TEXTURE_CUBE_MAP);

	if (setupPrograms() || setupTextures()) {
		glfwTerminate();
		return -1;
	}

#ifdef BULLET
	bulletInit();
#endif

	camera.setup();
	environment.setup();
	setupObjects();
	status.run = true;
	status.mode = status_t::CameraMode;

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	glfwSetScrollCallback(window, scrollCB);
	glfwSetCursorPosCallback(window, cursorCB);
	refreshCB(window);

	float past = status.animation = glfwGetTime();
	unsigned int count = 0;
	while (!glfwWindowShouldClose(window)) {
		render();

		// Step simulation
		step();

		// FPS counter
		count++;
		float now = glfwGetTime();
		if (now - past > 3) {
			float fps = (float)count / (now - past);
			char buf[32];
			sprintf(buf, "%g FPS [%s]", fps, status.mode == status_t::CameraMode ? "Camera" : "Tour");
			glfwSetWindowTitle(window, buf);
			count = 0;
			past = now;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	quit();
	return 0;
}
