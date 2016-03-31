#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <btBulletDynamicsCommon.h>
#include "world.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "global.h"
#include "helper.h"
#include "sphere.h"
#include "cube.h"
#include "wavefront.h"

//#define BULLET
//#define MODELS

#define CAMERA_POSITION	3.f
#define CAMERA_MOVEMENT	0.05f
#define CAMERA_ROTATE	(2.f * PI / 180.f)
#define WORLD_ROTATE	(2.f * PI / 180.f)

#define PI		(glm::pi<GLfloat>())
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

using namespace std;
using namespace glm;

GLFWwindow *window;

program_t programs[PROGRAM_COUNT];
texture_t textures[TEXTURE_COUNT];

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

static struct Camera {
	vec3 position;
	quat rotation;
	vec3 direction() {return rotation * vec3(0.f, 0.f, -1.f);}
	vec3 upward() {return rotation * vec3(0.f, 1.f, 0.f);}
} camera;

static struct Status {
	bool run;
	enum {WorldMode = 0, CameraMode} mode;
} status;

#ifdef BULLET
static struct Arena {
	Object *object;
	float scale;
	vec4 colour;
	vector<btRigidBody *> bodies;
} arena;
#endif

#ifdef MODELS
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
#else
Object *object;
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
	object = new Wavefront("models/simple.obj");
	//object = new Wavefront("models/nanoMiku.obj");
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
void scene()
{
	GLint *uniforms;

	// Render solid objects
	glUseProgram(programs[PROGRAM_WAVEFRONT].id);
	uniforms = programs[PROGRAM_WAVEFRONT].uniforms;

#if 0
	vec3 light(0.f, 0.f, 1.f);	// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
#else
	vec3 light(0.f, 1.f, 0.f);	// Light directionr
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
#endif
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position, 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

#ifndef MODELS
	object->bind();
	matrix.model = mat4();
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	object->renderSolid();
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

static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix.view = lookAt(camera.position, camera.position + camera.direction(), camera.upward());

	scene();
}

static void refreshCB(GLFWwindow *window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//GLfloat asp = (GLfloat)height / (GLfloat)width;
	//matrix.projection = ortho<GLfloat>(-1.f, 1.f, -asp, asp, -10, 10);
	matrix.projection = perspective<GLfloat>(45.f, (GLfloat)width / (GLfloat)height, 0.1f, 100.f);
}

static void keyCB(GLFWwindow */*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	switch (key) {
	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_Q:
		quit();
		return;
	case GLFW_KEY_M:
		if (status.mode == Status::WorldMode) {
			status.mode = Status::CameraMode;
			glfwSetWindowTitle(window, "Changed to camera mode");
		} else {
			status.mode = Status::WorldMode;
			glfwSetWindowTitle(window, "Changed to world mode");
		}
		return;
	case GLFW_KEY_SPACE:
		status.run = !status.run;
		return;
	}

	if (status.mode == Status::WorldMode)
		goto worldMode;

	switch (key) {
	case GLFW_KEY_K:	// Look upward
		camera.rotation = rotate(camera.rotation, CAMERA_ROTATE, vec3(1.f, 0.f, 0.f));
		break;
	case GLFW_KEY_J:	// Look downward
		camera.rotation = rotate(camera.rotation, -CAMERA_ROTATE, vec3(1.f, 0.f, 0.f));
		break;
	case GLFW_KEY_H:
		camera.rotation = rotate(camera.rotation, CAMERA_ROTATE, vec3(0.f, 1.f, 0.f));
		break;
	case GLFW_KEY_L:
		camera.rotation = rotate(camera.rotation, -CAMERA_ROTATE, vec3(0.f, 1.f, 0.f));
		break;
	case GLFW_KEY_UP:
		camera.position += camera.rotation * vec3(0.f, CAMERA_MOVEMENT, 0.f);
		break;
	case GLFW_KEY_DOWN:
		camera.position -= camera.rotation * vec3(0.f, CAMERA_MOVEMENT, 0.f);
		break;
	case GLFW_KEY_LEFT:
		camera.position -= camera.rotation * vec3(CAMERA_MOVEMENT, 0.f, 0.f);
		break;
	case GLFW_KEY_RIGHT:
		camera.position += camera.rotation * vec3(CAMERA_MOVEMENT, 0.f, 0.f);
		break;
	case GLFW_KEY_PAGE_UP:	// Move forward
		camera.position += camera.direction() * CAMERA_MOVEMENT;
		break;
	case GLFW_KEY_PAGE_DOWN:	// Move backward
		camera.position -= camera.direction() * CAMERA_MOVEMENT;
		break;
	case GLFW_KEY_R:
#ifdef BULLET
		camera.position = vec3(0.f, 0.f, arena.scale + CAMERA_POSITION);
#else
		camera.position = vec3(0.f, 0.f, 1.f + CAMERA_POSITION);
#endif
		camera.rotation = quat();
	}
	return;

worldMode:
	vec3 rot;
	switch (key) {
	case GLFW_KEY_UP:
		rot = vec3(1.f, 0.f, 0.f);
		break;
	case GLFW_KEY_DOWN:
		rot = vec3(-1.f, 0.f, 0.f);
		break;
	case GLFW_KEY_LEFT:
		rot = vec3(0.f, 1.f, 0.f);
		break;
	case GLFW_KEY_RIGHT:
		rot = vec3(0.f, -1.f, 0.f);
		break;
	case GLFW_KEY_PAGE_UP:
		rot = vec3(0.f, 0.f, 1.f);
		break;
	case GLFW_KEY_PAGE_DOWN:
		rot = vec3(0.f, 0.f, -1.f);
		break;
	case GLFW_KEY_R:
		matrix.world.rotation = quat();
	default:
		return;
	}
	matrix.world.rotation = rotate(quat(), -WORLD_ROTATE, rot) * matrix.world.rotation;
}

void setupUniforms(GLuint index)
{
	static const char *names[UNIFORM_COUNT] = {
		[UNIFORM_MVP]		= "mvpMatrix",
		[UNIFORM_MODEL]		= "modelMatrix",
		[UNIFORM_NORMAL]	= "normalMatrix",
		[UNIFORM_AMBIENT]	= "ambient",
		[UNIFORM_DIFFUSE]	= "diffuse",
		[UNIFORM_SPECULAR]	= "specular",
		[UNIFORM_SHININESS]	= "shininess",
		[UNIFORM_VIEWER]	= "viewer",
		[UNIFORM_LIGHT]		= "light",
		[UNIFORM_COLOUR]	= "colour",
	};
	GLuint program = programs[index].id;
	if (!program)
		return;
	for (GLuint idx = 0; idx < UNIFORM_COUNT; idx++)
		programs[index].uniforms[idx] = glGetUniformLocation(program, names[idx]);
}

GLuint setupPrograms()
{
	static const shader_t shaders[PROGRAM_COUNT][3] = {
		[PROGRAM_BASIC] = {
			{GL_VERTEX_SHADER, "basic.vert"},
			{GL_FRAGMENT_SHADER, "basic.frag"},
			{0, NULL}
		},
		[PROGRAM_LIGHTING] = {
			{GL_VERTEX_SHADER, "lighting.vert"},
			{GL_FRAGMENT_SHADER, "lighting.frag"},
			{0, NULL}
		},
		[PROGRAM_TEXTURE] = {
			{GL_VERTEX_SHADER, "texture.vert"},
			{GL_FRAGMENT_SHADER, "texture.frag"},
			{0, NULL}
		},
		[PROGRAM_WAVEFRONT] = {
			{GL_VERTEX_SHADER, "wavefront.vert"},
			{GL_FRAGMENT_SHADER, "wavefront.frag"},
			{0, NULL}
		},
	};

	for (GLuint idx = 0; idx < PROGRAM_COUNT; idx++) {
		GLuint program = glCreateProgram();
		if (program == 0)
			return 1;
		programs[idx].id = program;
		glBindAttribLocation(program, ATTRIB_POSITION, "position");
		glBindAttribLocation(program, ATTRIB_NORMAL, "normal");
		glBindAttribLocation(program, ATTRIB_TEXCOORD, "texCoord");
		if (setupProgramFromFiles(program, shaders[idx]) != 0)
			return 2;
		setupUniforms(idx);
	}
	return 0;
}

GLuint setupTextures()
{
	const static char *files[TEXTURE_COUNT] = {
		[TEXTURE_SPHERE] = "earth.jpg",
		[TEXTURE_S2] = "firemap.png",
		[TEXTURE_CUBE] = "diamond_block.png",
	};

	glActiveTexture(GL_TEXTURE0);
	for (GLuint i = 0; i < TEXTURE_COUNT; i++) {
		texture_t *tex = &textures[i];
		unsigned char *data = stbi_load(files[i], &tex->x, &tex->y, &tex->n, 3);
		if (data == 0) {
			cerr << "Error load texture file " << files[i] << endl;
			return 1;
		}
		if (tex->n != 3) {
			cerr << "Invalid image format from texture file " << files[i] << endl;
			stbi_image_free(data);
			return 2;
		}
		glGenTextures(1, &tex->texture);
		glBindTexture(GL_TEXTURE_2D, tex->texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex->x, tex->y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	return 0;
}

void quit()
{
	glfwTerminate();

	// Free memory
#ifdef MODELS
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
	glfwSwapInterval(0);
	glewExperimental = GL_TRUE;
	glewInit();

	if (setupPrograms()) {
		glfwTerminate();
		return -1;
	}

	if (setupTextures()) {
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef BULLET
	bulletInit();
#endif

	setupVertices();
	status.run = true;
	status.mode = Status::WorldMode;
#ifdef BULLET
	camera.position = vec3(0.f, 0.f, arena.scale + CAMERA_POSITION);
#else
	camera.position = vec3(0.f, 0.f, 1.f + CAMERA_POSITION);
#endif
	camera.rotation = quat();

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	refreshCB(window);

	float past = glfwGetTime();
#ifdef BULLET
	double animation = glfwGetTime();
#endif
	unsigned int count = 0;
	while (!glfwWindowShouldClose(window)) {
		render();

#ifdef BULLET
		// Step simulation
		if (status.run) {
			double time = glfwGetTime();
			dynamicsWorld->stepSimulation(time - animation, 100);
			animation = time;
		}
#endif

		// FPS counter
		count++;
		float now = glfwGetTime();
		if (now - past > 3) {
			float fps = (float)count / (now - past);
			char buf[32];
			sprintf(buf, "%g FPS [%s]", fps, status.mode == Status::CameraMode ? "Camera" : "World");
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
