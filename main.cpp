#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <cstdlib>

#include <btBulletDynamicsCommon.h>
#include "world.h"

#include "global.h"
#include "helper.h"
#include "camera.h"

#include "sphere.h"
#include "cube.h"
#include "cubeenclosed.h"
#include "skybox.h"
#include "wavefront.h"

using namespace std;
using namespace glm;

GLFWwindow *window;

program_t programs[PROGRAM_COUNT];
texture_t textures[TEXTURE_COUNT];

environment_t environment;

Camera camera;

static struct {
	mat4 model, view, projection;
	mat4 mvp;
	mat3 normal;

	struct {
		quat rotation;
	} world;

	void update()
	{
		model = mat4_cast(world.rotation) * model;
		mvp = projection * view * model;
		normal = mat3(transpose(inverse(view * model)));
	}
} matrix;

static struct Status {
	bool run;
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
Skybox *skybox;
//Object *object;
vector<Object *> objects;
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

void setupVertices()
{
#ifndef MODELS
	skybox = new Skybox;
	//object = new Wavefront("models/simple.obj", "models/", "models/");
	objects.push_back(new Wavefront("models/nanoMiku/nanoMiku.obj", "models/nanoMiku/", "models/nanoMiku/"));
	objects.push_back(new Wavefront("models/arena/arena_01.obj", "models/arena/", "models/arena/textures/"));
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
static void renderSkybox()
{
	glDepthMask(GL_FALSE);
	// Render solid objects
	glUseProgram(programs[PROGRAM_SKYBOX].id);
	uniformMap &uniforms = programs[PROGRAM_SKYBOX].uniforms;

	// Material properties
	glUniform3fv(uniforms[UNIFORM_AMBIENT], 1, (GLfloat *)&environment.ambient);

	// Render skybox
	matrix.model = translate(mat4(), camera.position());
	//matrix.model = mat4();
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);

	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SKYBOX].texture);
	skybox->bind();
	skybox->render();
	glDepthMask(GL_TRUE);
}

static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix.view = camera.view();

	renderSkybox();

	// Render solid objects
	glUseProgram(programs[PROGRAM_WAVEFRONT].id);
	uniformMap &uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

	vec3 light(0.f, 0.f, 1.f);		// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	vec3 intensity(0.5f, 0.3f, 0.2f);	// Light intensity
	glUniform3fv(uniforms[UNIFORM_LIGHT_INTENSITY], 1, (GLfloat *)&intensity);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position(), 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

#ifndef MODELS
	matrix.model = mat4();
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	for (Object *object: objects) {
		object->bind();
		object->render();
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
	matrix.projection = perspective<GLfloat>(45.f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.f);
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
		return;
	case GLFW_KEY_R:
		// Reset all animation (optional)
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

void cursorCB(GLFWwindow *window, double xpos, double ypos)
{
	camera.cursorCB(xpos, ypos);
}

void quit()
{
	glfwTerminate();

	// Free memory
#ifndef MODELS
	delete skybox;
	for (Object *object: objects)
		delete object;
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

	if (setupPrograms() || setupTextures()) {
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef BULLET
	bulletInit();
#endif

	setupVertices();
	status.run = true;
	status.mode = Status::CameraMode;

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	glfwSetCursorPosCallback(window, cursorCB);
	refreshCB(window);

	float past = glfwGetTime();
	double animation = glfwGetTime();
	unsigned int count = 0;
	while (!glfwWindowShouldClose(window)) {
		render();

		// Step simulation
		if (status.run) {
			double time = glfwGetTime();
#ifdef BULLET
			dynamicsWorld->stepSimulation(time - animation, 100);
#endif
			camera.updateCB(time - animation);
			animation = time;
		}

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
