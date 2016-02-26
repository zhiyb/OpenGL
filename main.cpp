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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "global.h"
#include "helper.h"
#include "sphere.h"
#include "cube.h"

#define SCENE_NUM	5
#define SCENE_SCALE	0.7f
#define CAMERA_MOVEMENT	0.05f
#define CAMERA_ROTATE	(2.f * PI / 180.f)
#define WORLD_ROTATE	(2.f * PI / 180.f)

#define PI		(glm::pi<GLfloat>())
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

using namespace std;
using namespace glm;

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
		model = mat4(world.rotation) * model;
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
	int scene;
	enum {CameraMode = 0, WorldMode} mode;
} status;

static Sphere *sphere;

struct Model {
	struct Data {
		vec4 colour;
		vec3 scale;
		struct Animation {
			vec3 centre, normal;
			vec3 initpos;	// Relative to centre
			GLfloat speed;
		} animation;
	};
	vector<Data> data;
	Object *object;
};
vector<Model *> models;

void setupModelData(Model *model, const Model::Data *data, int size)
{
	while (size--)
		model->data.push_back(*data++);
}

void setupVertices()
{
	sphere = new Sphere(32);
	sphere->setup();

	Model *model;

	static const Model::Data sphereModels[] = {
		// colour, scale, {centre, normal, initpos, speed,},
		{vec4(1.f, 0.f, 0.f, 1.f), vec3(0.25f),
		 {vec3(0.f), vec3(1.f, 0.f, 0.f), vec3(0.f, 0.8f, 0.f), 0.25f,},},
		{vec4(0.f, 1.f, 0.f, 1.f), vec3(0.125f),
		 {vec3(0.f), vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 0.8f), 0.5f,},},
		{vec4(0.f, 0.f, 1.f, 1.f), vec3(0.0625f),
		 {vec3(0.f), vec3(0.f, 0.f, 1.f), vec3(0.8f, 0.f, 0.f), 1.f,},},
	};

	model = new Model;
	model->object = new Sphere(32);
	model->object->setup();
	setupModelData(model, sphereModels, ARRAY_SIZE(sphereModels));
	models.push_back(model);

	static const Model::Data cubeModels[] = {
		// colour, scale, {centre, normal, initpos, speed,},
		{vec4(0.2f, 0.4f, 1.f, 1.f), vec3(0.2f),
		 {vec3(0.5f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), 0.3f,},},
		{vec4(1.f, 1.f, 0.f, 1.f), vec3(0.1f),
		 {vec3(-0.5f, 0.f, 0.f), vec3(1.f, 0.f, 0.f), vec3(0.f, 0.5f, 0.f), 0.2f,},},
	};

	model = new Model;
	model->object = new Cube();
	model->object->setup();
	setupModelData(model, cubeModels, ARRAY_SIZE(cubeModels));
	models.push_back(model);
}

void sceneA()
{
	// (A) Draw a wire-frame sphere by calculating all
	// the vertex positions and drawing lines between them.
	glUseProgram(programs[PROGRAM_BASIC].id);
	GLint *uniforms = programs[PROGRAM_BASIC].uniforms;

	glUniform4f(uniforms[UNIFORM_COLOUR], 0.1f, 0.2f, 1.f, 1.f);

	matrix.model = scale(mat4(), vec3(SCENE_SCALE));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);

	sphere->bind();
	sphere->renderFrame();
}

void sceneB()
{
	// (B) For the sphere, work out the surface normal direction and
	// augment your wire-frame drawing with short lines representing
	// the normal direction of each vertex.
	// The sphere should now appear to be a hedgehog.
	sceneA();
	GLint *uniforms = programs[PROGRAM_BASIC].uniforms;
	glUniform4f(uniforms[UNIFORM_COLOUR], 1.f, 0.f, 0.f, 1.f);
	sphere->renderNormal();
}

void sceneC()
{
	// (C) Use the normal information calculated in (b) above work out
	// the amount of illumination falling on each vertex.
	// Assume that the light source is at infinity and is co-axial
	// with the viewpoint. Use this to draw a shaded sphere.
	glUseProgram(programs[PROGRAM_LIGHTING].id);
	GLint *uniforms = programs[PROGRAM_LIGHTING].uniforms;

	vec3 light(0.f, 0.f, 1.f);	// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position, 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

	// Material properties
	glUniform1f(uniforms[UNIFORM_AMBIENT], 0.3f);
	glUniform1f(uniforms[UNIFORM_DIFFUSE], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULAR], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULARPOWER], 13.f);

	vec4 colour(0.f, 0.f, 1.f, 1.f);
	glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&colour);

	matrix.model = scale(mat4(), vec3(0.8));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	sphere->bind();
	sphere->renderSolid();
}

void sceneD()
{
	// (D) Develop a simple animation showing a number of cones and spheres
	// moving along regular paths. These can be wireframe or solid.
	glUseProgram(programs[PROGRAM_LIGHTING].id);
	GLint *uniforms = programs[PROGRAM_LIGHTING].uniforms;

	vec3 light(0.f, 0.f, 1.f);	// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(light, 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position, 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

	// Material properties
	glUniform1f(uniforms[UNIFORM_AMBIENT], 0.3f);
	glUniform1f(uniforms[UNIFORM_DIFFUSE], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULAR], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULARPOWER], 13.f);

	float time = glfwGetTime();
	for (Model *model: models) {
		model->object->bind();
		for (Model::Data &data: model->data) {
			// Calculate model matrix based on animation data
			Model::Data::Animation &ani = data.animation;
			GLfloat angle = time * ani.speed * PI * 2;
			matrix.model = mat4();
			matrix.model = translate(matrix.model, ani.centre);
			matrix.model = rotate(matrix.model, angle, ani.normal);
			matrix.model = translate(matrix.model, ani.initpos);
			matrix.model = scale(matrix.model, data.scale);

			matrix.update();
			glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
			glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

			glUniform4fv(uniforms[UNIFORM_COLOUR], 1, (GLfloat *)&data.colour);
			model->object->renderSolid();
		}
	}
}

void sceneE()
{
	// (E) Draw a textured object, such as a rectangle (plane), box or sphere.
	glUseProgram(programs[PROGRAM_TEXTURE].id);
	GLint *uniforms = programs[PROGRAM_TEXTURE].uniforms;

	vec3 light(1.f, 0.f, 1.f);	// Light direction
	light = vec3(transpose(inverse(matrix.view)) * vec4(normalize(light), 0.f));
	glUniform3fv(uniforms[UNIFORM_LIGHT], 1, (GLfloat *)&light);
	vec3 viewer = vec3(transpose(inverse(matrix.view)) * vec4(camera.position, 0.f));
	glUniform3fv(uniforms[UNIFORM_VIEWER], 1, (GLfloat *)&viewer);

	// Material properties
	glUniform1f(uniforms[UNIFORM_AMBIENT], 0.3f);
	glUniform1f(uniforms[UNIFORM_DIFFUSE], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULAR], 1.f);
	glUniform1f(uniforms[UNIFORM_SPECULARPOWER], 13.f);

	matrix.model = scale(mat4(), vec3(0.8));
	matrix.update();
	glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, (GLfloat *)&matrix.mvp);
	glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, (GLfloat *)&matrix.model);
	glUniformMatrix3fv(uniforms[UNIFORM_NORMAL], 1, GL_FALSE, (GLfloat *)&matrix.normal);

	texture_t *tex = &textures[TEXTURE_SPHERE];
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex->x, tex->y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	sphere->bind();
	sphere->renderSolid();
}

static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix.view = lookAt(camera.position, camera.position + camera.direction(), camera.upward());
	//matrix.view = matrix.view * mat4(matrix.world.rotation);

	void (*scenes[])() = {sceneA, sceneB, sceneC, sceneD, sceneE};
	(*scenes[status.scene])();
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
		glfwTerminate();
		exit(0);
		return;
	case GLFW_KEY_M:
		status.mode = status.mode == Status::WorldMode ? Status::CameraMode : Status::WorldMode;
		return;
	case GLFW_KEY_A ... (GLFW_KEY_A + SCENE_NUM - 1):
		status.scene = key - GLFW_KEY_A;
		return;
	case GLFW_KEY_SPACE:
		status.scene = (status.scene + 1) % SCENE_NUM;
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
		camera.position = vec3(0.f, 0.f, 2.f);
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
		[UNIFORM_SPECULARPOWER]	= "specularPower",
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
	};

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
		glGenBuffers(1, &tex->pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, tex->x * tex->y * tex->n, data, GL_STATIC_DRAW);
		stbi_image_free(data);
	}

	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return 0;
}

int main(int /*argc*/, char */*argv*/[])
{
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow *window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -2;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	glewInit();

	if (setupPrograms()) {
		glfwTerminate();
		return -3;
	}

	if (setupTextures()) {
		glfwTerminate();
		return -4;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setupVertices();
	status.scene = 0;
	status.mode = Status::CameraMode;
	camera.position = vec3(0.f, 0.f, 2.f);
	camera.rotation = quat();

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	refreshCB(window);

	float past = glfwGetTime();
	unsigned int count = 0;
	while (!glfwWindowShouldClose(window)) {
		render();
		glfwSwapBuffers(window);

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

		glfwPollEvents();
	}

	glfwTerminate();
	// TODO: Free the models?
	return 0;
}
