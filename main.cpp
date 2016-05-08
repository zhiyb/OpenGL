#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>

#include "helper.h"
#include "world.h"
#include "global.h"
#include "camera.h"
#include "bullet.h"

#include "sphere.h"
#include "square.h"
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

struct shadow_t {
	struct env_t {
		GLuint texture;
	} environment;
	GLuint fbo;
} shadow;

Square *square;

struct object_t {
	string name;
	bool culling;
	vec3 scale, offset;
	bool bullet;
	float mass;
	btRigidBody *rigidBody;
	Object *model;
};
vector<object_t> objects;

void quit();

static void setupShadowStorage()
{
	glGenTextures(1, &shadow.environment.texture);
	glBindTexture(GL_TEXTURE_2D, shadow.environment.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
		     SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glGenFramebuffers(1, &shadow.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupObjects()
{
	square = new Square;

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
		ss >> obj.name >> modelPath >> mtlPath >> texPath;
		if (!ss)
			continue;
		ss >> obj.culling >> obj.scale >> obj.offset;
		ss >> obj.bullet >> obj.mass;
		clog << __func__ << ": Model " << modelPath << " loading..." << endl;
		Wavefront *model = new Wavefront(modelPath.c_str(), mtlPath.c_str(), texPath.c_str());
		if (!model)
			continue;
		if (!model->isValid()) {
			delete model;
			continue;
		}
		obj.model = model;

		obj.rigidBody = 0;
		if (!obj.bullet) {
			objects.push_back(obj);
			continue;
		}

		if (!obj.mass) {	// Stationary object
			vector<btRigidBody *> rigidBodies;
			model->createStaticRigidBody(&rigidBodies, to_btVector3(obj.scale));
			for (btRigidBody *rigidBody: rigidBodies) {
				btVector3 origin = rigidBody->getWorldTransform().getOrigin();
				rigidBody->getWorldTransform().setOrigin(origin + to_btVector3(obj.offset));
				bulletAddRigidBody(rigidBody, BULLET_GROUND);
			}
		} else {		// Use bounding box
			obj.rigidBody = model->createRigidBody(to_btVector3(obj.scale), obj.mass);
			btVector3 origin = obj.rigidBody->getWorldTransform().getOrigin();
			obj.rigidBody->getWorldTransform().setOrigin(origin + to_btVector3(obj.offset));
			bulletAddRigidBody(obj.rigidBody, BULLET_GROUND);
			obj.offset = -model->boundingOrigin() * obj.scale;
		}
		objects.push_back(obj);
	}
}

static void renderObjects()
{
	glUseProgram(programs[PROGRAM_WAVEFRONT].id);
	//checkError("switching to PROGRAM_WAVEFRONT");
	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	glUniform1i(uniforms[UNIFORM_SAMPLER], 0);
	glUniform1i(uniforms[UNIFORM_SAMPLER_SHADOW], 1);
	glUniformMatrix4fv(uniforms[UNIFORM_MAT_SHADOW], 1, GL_FALSE, (GLfloat *)&shadowMatrix.mvp);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, shadow.environment.texture);
	glActiveTexture(GL_TEXTURE0);

	glUniform3fv(uniforms[UNIFORM_LIGHT_POSITION], 1, (GLfloat *)&environment.light.position);
	glUniform3fv(uniforms[UNIFORM_LIGHT_INTENSITY], 1, (GLfloat *)&environment.light.intensity);
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&camera.position());

	for (object_t &obj: objects) {
		matrix.model = translate(mat4(), obj.offset);
		if (obj.rigidBody)
			matrix.model = bulletGetMatrix(obj.rigidBody) * matrix.model;
		matrix.model = scale(matrix.model, obj.scale);
		matrix.update();
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
		glUniformMatrix3fv(uniforms[UNIFORM_MAT_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

		if (obj.culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		obj.model->bind();
		obj.model->render();
	}
}

static void renderEnvironmentShadow()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	shadowMatrix.projection = perspective<GLfloat>(45.f, (GLfloat)width / (GLfloat)height, 1.f, 20.f);
	shadowMatrix.view = lookAt(environment.light.position,
				   vec3(0.f), vec3(0.f, 1.f, 0.f));

	glUseProgram(programs[PROGRAM_SHADOW].id);
	uniformMap &uniforms = programs[PROGRAM_SHADOW].uniforms;
	//environment.renderShadow();

	for (object_t &obj: objects) {
		shadowMatrix.model = translate(mat4(), obj.offset);
		if (obj.rigidBody)
			shadowMatrix.model = bulletGetMatrix(obj.rigidBody) * shadowMatrix.model;
		shadowMatrix.model = scale(shadowMatrix.model, obj.scale);
		shadowMatrix.update();
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MVP], 1, GL_FALSE, (GLfloat *)&shadowMatrix.mvp);

		if (obj.culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		obj.model->bind();
		obj.model->render();
	}

	static const mat4 scaleBiasMatrix = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
						 vec4(0.0f, 0.5f, 0.0f, 0.0f),
						 vec4(0.0f, 0.0f, 0.5f, 0.0f),
						 vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shadowMatrix.mvp = scaleBiasMatrix * shadowMatrix.projection * shadowMatrix.view;
}

static void render()
{
	// Render shadow map to texture
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow.environment.texture, 0);
	glViewport(0, 0, SHADOW_TEXTURE_SIZE, SHADOW_TEXTURE_SIZE);
	glClear(GL_DEPTH_BUFFER_BIT);
	// Resolve depth-fighting issues
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.f, 4.f);
	renderEnvironmentShadow();
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Render shadow texture to screen
	if (status.mode == status_t::EnvShadowMode) {
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs[PROGRAM_TEXTURE_BASIC].id);
		uniformMap &uniforms = programs[PROGRAM_TEXTURE_BASIC].uniforms;
		mat4 mat = rotate(mat4(), -PI / 2.f, vec3(1.f, 0.f, 0.f));
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MVP], 1, GL_FALSE, (GLfloat *)&mat);
		glUniform3f(uniforms[UNIFORM_AMBIENT], 1.f, 1.f, 1.f);

		glBindTexture(GL_TEXTURE_2D, shadow.environment.texture);
		square->bind();
		square->render();
		return;
	}

	// Render scene
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	environment.render();
	camera.render();
	renderObjects();
}

void step(bool first = false)
{
	double time = glfwGetTime();
	double diff = time - status.animation;
	status.animation = time;
	camera.updateCB(diff);

	if (first || (status.run && diff > 0.f)) {
		//clog << __func__ << ": " << diff << endl;
		time -= status.pauseDuration;
		// Step 1 bullet simulation frame at first time
		bulletUpdate(first ? 0.f : diff);
		environment.update(time);
	}
}

void tour(const bool e)
{
}

void report()
{
	clog << "****** Report at " << glfwGetTime() << " seconds ******" << endl;
	environment.print();
	camera.print();
	for (object_t &obj: objects) {
		if (!obj.rigidBody)
			continue;
		btTransform trans;
		obj.rigidBody->getMotionState()->getWorldTransform(trans);
		vec3 pos = from_btVector3(trans.getOrigin()) - ((Wavefront *)obj.model)->boundingOrigin() * obj.scale;
		quat rot = from_btQuaternion(trans.getRotation());
		clog << obj.name << "\t@" << pos << ", " << rot << endl;
	}
}

static void refreshCB(GLFWwindow *window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//GLfloat asp = (GLfloat)height / (GLfloat)width;
	//matrix.projection = ortho<GLfloat>(-1.f, 1.f, -asp, asp, -10, 10);
	matrix.projection = perspective<GLfloat>(45.f, (GLfloat)width / (GLfloat)height, 0.02f, 30.f);
}

static void keyCB(GLFWwindow */*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	switch (key) {
	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_Q:
		// Exit the program
		quit();
		return;
#ifndef SUBMISSION
	case GLFW_KEY_X:
		report();
		return;
#endif
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

	case GLFW_KEY_P:
		// Move to predefined location (screen shot)
		camera.backup();
		loadRecord(records["P"]);
		return;
	case GLFW_KEY_L:
		// Alternative view point 1 (optional)
		camera.backup();
		loadRecord(records["L"]);
		return;
	case GLFW_KEY_O:
		// Alternative view point 2 (overhead, optional)
		camera.backup();
		loadRecord(records["O"]);
		return;
	case GLFW_KEY_M:
		// Return to last position of mobile camera
		camera.restore();
		return;

#ifndef SUBMISSION
	case GLFW_KEY_0:
		if (status.mode == status_t::EnvShadowMode)
			status.mode = status_t::CameraMode;
		else
			status.mode = status_t::EnvShadowMode;
		return;
#endif
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

	loadRecords();
	camera.setup();
	environment.setup();
	setupObjects();
	setupShadowStorage();

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	glfwSetScrollCallback(window, scrollCB);
	glfwSetCursorPosCallback(window, cursorCB);
	refreshCB(window);

	float past = status.animation = glfwGetTime();
	unsigned int count = 0;
	bool first = true;
	while (!glfwWindowShouldClose(window)) {
		step(first);
		first = false;
		render();

		// FPS counter
		count++;
		float now = glfwGetTime();
		if (now - past > 1.f) {
			float fps = (float)count / (now - past);
			count = 0;
			past = now;

			ostringstream ss;
			ss << fps << " FPS [";
			if (status.mode == status_t::CameraMode)
				ss << "Camera";
			else if (status.mode == status_t::TourMode)
				ss << "Tour";
			else if (status.mode == status_t::EnvShadowMode)
				ss << "Shadow: Environment";
			ss << "]";
			if (!status.run)
				ss << " *Paused*";
			glfwSetWindowTitle(window, ss.str().c_str());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	quit();
	return 0;
}
