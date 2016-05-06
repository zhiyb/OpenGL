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

#include "sphere.h"
#include "cube.h"
#include "cubeenclosed.h"
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

static struct Status {
	bool run;
	double animation, pauseStart, pauseDuration;
	enum {CameraMode, TourMode} mode;
} status;

#ifdef BULLET
static struct Arena {
	Object *object;
	float scale;
	vec4 colour;
	vector<btRigidBody *> bodies;
} arena;
#endif

#ifndef MODELS
struct object_t {
	bool culling;
	vec3 scale, offset;
	Object *model;
};
vector<object_t> objects;
#else
struct Model {
	struct Data {
		enum {Wireframe, Solid, Textured} type;
		union {
			struct {
				GLfloat colour[4];
			};
			GLuint texture;
		};
		btScalar scale;
	};
	struct Init {
		struct Data modelData;
		btScalar restitution;
		btScalar mass;
		btVector3 position;
		btVector3 velocity;
	};
	vector<Data> data;
#ifdef BULLET
	vector<btRigidBody *> bodies;
#endif
	Object *object;
};
vector<Model *> models;
#endif

#ifdef BULLET
// Bullet physics
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

void bulletInit()
{
	/*
	 * set up world
	 */
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0., GRAVITY, 0));

	/*
	 * Set up arena
	 */
	arena.object = new Cube;
	arena.object->setup();
	arena.scale = 1.5f;
	arena.colour = vec4(0.2f, 0.4f, 1.f, 1.f);

	btVector3 normals[] = {
		btVector3(0.f, 0.f, 1.f),
		btVector3(0.f, 1.f, 0.f),
		btVector3(1.f, 0.f, 0.f),
		btVector3(0.f, 0.f, -1.f),
		btVector3(0.f, -1.f, 0.f),
		btVector3(-1.f, 0.f, 0.f),
	};
	for (unsigned int i = 0; i != 6; i++) {
		btCollisionShape* shape = new btStaticPlaneShape(normals[i], 0.f);
		btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), -arena.scale * normals[i]));
		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0,motionState,shape,btVector3(0,0,0));
		btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
		rigidBody->setRestitution(RESTITUTION);
		rigidBody->setFriction(FRICTION);
		dynamicsWorld->addRigidBody(rigidBody);
		arena.bodies.push_back(rigidBody);
	}
}

mat4 bulletStep(btRigidBody* rigidBody) {
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	mat4 matrix;
	trans.getOpenGLMatrix((btScalar *)&matrix);
	return matrix;
}
#endif

void quit();

#ifdef MODELS
void setupModelData(Model *model, const Model::Init *init, int size)
{
	while (size--) {
#ifdef BULLET
		btTransform t = btTransform(btQuaternion(0, 0, 0, 1), init->position);
		btRigidBody *rigidBody = model->object->createRigidBody(init->mass, init->modelData.scale, t);
		rigidBody->setRestitution(init->restitution);
		rigidBody->setFriction(FRICTION);
		rigidBody->setLinearVelocity(init->velocity);
		dynamicsWorld->addRigidBody(rigidBody);
		model->bodies.push_back(rigidBody);
#endif
		model->data.push_back(init->modelData);
		init++;
	}
}
#endif

void setupObjects()
{
#ifndef MODELS
	ifstream datafs(DATA_PATH "/wavefront.txt");
	if (!datafs) {
		cerr << "Cannot open model description file " DATA_PATH "wavefront.txt" << endl;
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
		ss >> obj.culling;
		ss >> obj.scale >> obj.offset;
		if (!ss)
			continue;
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
	}
#else
	Model *model;

	static const Model::Init sphereModels[] = {
		// modelData{type, {colour/texture}, scale},
		// restitution, mass, position, velocity
		{{Model::Data::Textured, {.texture = TEXTURE_S2}, 0.25f},
		 RESTITUTION, 0.25f, btVector3(0.f, 0.8f, 0.f), btVector3(0.f, 5.f, 0.f)},
		{{Model::Data::Wireframe, {0.f, 1.f, 0.f, 1.f}, 0.125f},
		 RESTITUTION, 0.125f, btVector3(0.f, 0.f, 0.8f), btVector3(0.f, 0.f, 5.f)},
		{{Model::Data::Solid, {0.f, 0.f, 1.f, 1.f}, 0.0625f},
		 RESTITUTION, 0.0625f, btVector3(0.8f, 0.f, 0.f), btVector3(5.f, 0.f, 0.f)},
	};

	model = new Model;
	model->object = new Sphere(32);
	model->object->setup();
	setupModelData(model, sphereModels, ARRAY_SIZE(sphereModels));
	models.push_back(model);

	static const Model::Init cubeModels[] = {
		// modelData{type, {colour/texture}, scale},
		// restitution, mass, position, velocity
		{{Model::Data::Textured, {.texture = TEXTURE_CUBE}, 0.2f},
		 RESTITUTION, 0.2f, btVector3(0.5f, 0.f, 0.f), btVector3(5.f, 0.f, 0.f)},
		{{Model::Data::Solid, {1.f, 1.f, 0.f, 1.f}, 0.1f},
		 RESTITUTION, 0.1f, btVector3(-0.5f, 0.f, 0.f), btVector3(-5.f, 0.f, 0.f)},
	};

	model = new Model;
	model->object = new Cube();
	model->object->setup();
	setupModelData(model, cubeModels, ARRAY_SIZE(cubeModels));
	models.push_back(model);
#endif
}

#if !defined(MODELS) || !defined(BULLET)
static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix.view = camera.view();
	environment.render();

	// Render solid objects
	glUseProgram(programs[PROGRAM_WAVEFRONT].id);
	checkError("switching to PROGRAM_WAVEFRONT");
	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	vec3 light = vec3(transpose(inverse(matrix.view)) * vec4(environment.light.direction, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT_DIRECTION], 1, (GLfloat *)&light);
	glUniform3fv(uniforms[UNIFORM_LIGHT_INTENSITY], 1, (GLfloat *)&environment.light.intensity);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position(), 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

#ifndef MODELS
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
#else
	for (Model *model: models) {
		model->object->bind();
		for (unsigned int i = 0; i != model->data.size(); i++) {
			Model::Data &data = model->data[i];

			if (data.type != Model::Data::Solid)
				continue;
			// Step model physics
			//matrix.model = bulletStep(model->bodies[i]);
			//matrix.model = scale(matrix.model, vec3(data.scale));
			matrix.model = mat4();

			matrix.update();
			glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
			glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

			glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&data.colour);
			model->object->renderSolid();
		}
	}
#endif
}
#else
void scene()
{
	// Render wireframes
	glUseProgram(programs[PROGRAM_BASIC].id);
	GLint *uniforms = programs[PROGRAM_BASIC].uniforms;

#if 1
	// Render arena frame
	matrix.model = scale(mat4(), vec3(arena.scale));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	vec4 colour(0.f, 0.f, 0.f, 1.f);
	glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&colour);
	arena.object->bind();
	arena.object->renderWireframe();
#endif

	for (Model *model: models) {
		model->object->bind();
		for (unsigned int i = 0; i != model->data.size(); i++) {
			Model::Data &data = model->data[i];

			if (data.type != Model::Data::Wireframe)
				continue;
			// Step model physics
			matrix.model = bulletStep(model->bodies[i]);
			matrix.model = scale(matrix.model, vec3(data.scale));

			matrix.update();
			glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
			glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

			glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&data.colour);
			model->object->renderWireframe();
		}
	}

	// Render solid objects
	glUseProgram(programs[PROGRAM_LIGHTING].id);
	uniforms = programs[PROGRAM_LIGHTING].uniforms;

	vec3 light(0.f, 0.f, 1.f);	// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position, 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

	// Material properties
	glUniform1f(uniforms[UNIFORM_AMBIENT], 0.3f);
	glUniform1f(uniforms[UNIFORM_DIFFUSE], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULAR], 0.5f);
	glUniform1f(uniforms[UNIFORM_SPECULARPOWER], 10.f);

#if 1
	// Render arena
	matrix.model = scale(mat4(), vec3(-arena.scale));
	matrix.update();
	matrix.normal = mat3(scale(mat4(matrix.normal), vec3(-1.f)));
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&arena.colour);
	arena.object->bind();
	arena.object->renderSolid();
#endif

	for (Model *model: models) {
		model->object->bind();
		for (unsigned int i = 0; i != model->data.size(); i++) {
			Model::Data &data = model->data[i];

			if (data.type != Model::Data::Solid)
				continue;
			// Step model physics
			matrix.model = bulletStep(model->bodies[i]);
			matrix.model = scale(matrix.model, vec3(data.scale));

			matrix.update();
			glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
			glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

			glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&data.colour);
			model->object->renderSolid();
		}
	}

	// Render textured objects
	glUseProgram(programs[PROGRAM_TEXTURE].id);
	uniforms = programs[PROGRAM_TEXTURE].uniforms;

	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

	// Material properties
	glUniform1f(uniforms[UNIFORM_AMBIENT], 0.3f);
	glUniform1f(uniforms[UNIFORM_DIFFUSE], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULAR], 0.5f);
	glUniform1f(uniforms[UNIFORM_SPECULARPOWER], 10.f);

#if 0
	// Render textured arena
	matrix.model = scale(mat4(), vec3(-arena.scale));
	matrix.update();
	matrix.normal = mat3(scale(mat4(matrix.normal), vec3(-1.f)));
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_CUBE].texture);
	arena.object->bind();
	arena.object->renderSolid();
#endif

	for (Model *model: models) {
		model->object->bind();
		for (unsigned int i = 0; i != model->data.size(); i++) {
			Model::Data &data = model->data[i];

			if (data.type != Model::Data::Textured)
				continue;
			// Step model physics
			matrix.model = bulletStep(model->bodies[i]);
			matrix.model = scale(matrix.model, vec3(data.scale));

			matrix.update();
			glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
			glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

			glBindTexture(GL_TEXTURE_2D, textures[data.texture].texture);
			model->object->renderSolid();
		}
	}
}
#endif

void step()
{
	if (status.run) {
		double time = glfwGetTime() - status.pauseDuration;
		double diff = time - status.animation;
#ifdef BULLET
		dynamicsWorld->stepSimulation(diff, 100);
#endif
		environment.update(time);
		camera.updateCB(diff);
		status.animation = time;
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

	if (status.mode == Status::TourMode) {
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
		status.run = !status.run;
		if (status.run)
			status.pauseDuration += glfwGetTime() - status.pauseStart;
		else
			status.pauseStart = glfwGetTime();
		return;
	case GLFW_KEY_R:
		// Reset all animation (optional)
		status.pauseDuration = glfwGetTime();
		status.run = true;
		return;
	case GLFW_KEY_P:
		// Move to predefined location (screen shot)
		camera.backup();
		camera.setPosition(CAMERA_V0_POS);
		camera.setRotation(CAMERA_V0_ROT);
		camera.setSpeed(0.f);
		return;
	case GLFW_KEY_L:
		// Alternative view point 1 (optional)
		camera.backup();
		camera.setPosition(CAMERA_V1_POS);
		camera.setRotation(CAMERA_V1_ROT);
		camera.setSpeed(0.f);
		return;
	case GLFW_KEY_O:
		// Alternative view point 2 (overhead, optional)
		camera.backup();
		camera.setPosition(CAMERA_V2_POS);
		camera.setRotation(CAMERA_V2_ROT);
		camera.setSpeed(0.f);
		return;
	case GLFW_KEY_M:
		// Return to last position of mobile camera
		camera.restore();
		return;
	}

	camera.keyCB(key);
}

void mouseCB(GLFWwindow */*window*/, int button, int action, int /*mods*/)
{
	camera.mouseCB(button, action);
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
#ifdef BULLET
	for (btRigidBody *body: arena.bodies) {
		dynamicsWorld->removeRigidBody(body);
		delete body;
	}
#endif
#ifdef BULLET
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
#endif
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

	environment.setup();
	environment.load();
	setupObjects();
	status.run = true;
	status.mode = Status::CameraMode;

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	//glfwSetScrollCallback(window, scrollCB);
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
			sprintf(buf, "%g FPS [%s]", fps, status.mode == Status::CameraMode ? "Camera" : "Tour");
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
