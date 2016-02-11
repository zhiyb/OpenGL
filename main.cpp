#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "global.h"
#include "helper.h"
#include "sphere.h"
#include "cube.h"

#define FILE_VERTEX_SHADER	"light_frag.vert"
#define FILE_FRAGMENT_SHADER	"light_frag.frag"

#define SCENE_NUM	6
#define SCENE_SCALE	0.7f

using namespace std;
using namespace glm;

map<string, GLint> location;
static struct {
	mat4 model, view, projection;
} matrix;

static struct {
	int scene;
} status;

struct Model {
	struct Position {
		mat4 model;
	};
	vector<Position> position;
	Object *object;
};
vector<Model *> models;

static Object *sphere;

void setupVertices()
{
	sphere = new Sphere(48);
	sphere->setup();

	Model *model;
	Model::Position pos;

	model = new Model;
	model->object = new Sphere(32);
	model->object->setup();
	pos.model = scale(translate(mat4(), vec3(-0.5f, -0.5f, -0.5f)), vec3(0.2f));
	model->position.push_back(pos);
	pos.model = scale(translate(mat4(), vec3(-0.5f, -0.5f, 0.5f)), vec3(0.2f));
	model->position.push_back(pos);
	models.push_back(model);

	model = new Model;
	model->object = new Cube;
	model->object->setup();
	pos.model = scale(translate(mat4(), vec3(0.5f, -0.5f, -0.5f)), vec3(0.2f));
	model->position.push_back(pos);
	pos.model = scale(translate(mat4(), vec3(0.5f, -0.5f, 0.5f)), vec3(0.2f));
	model->position.push_back(pos);
	models.push_back(model);
}

void updateMatrices()
{
	mat4 mvpMatrix = matrix.projection * matrix.view * matrix.model;
	glUniformMatrix4fv(location["mvpMatrix"], 1, GL_FALSE, (GLfloat *)&mvpMatrix);
	mat4 normalMatrix = transpose(inverse(matrix.view * matrix.model));
	glUniformMatrix4fv(location["normalMatrix"], 1, GL_FALSE, (GLfloat *)&normalMatrix);
}

void updateModel(const vec4 colour, const vec3 pos, const vec3 scale)
{
	glUniform4fv(location["colour"], 1, (GLfloat *)&colour);
	matrix.model = glm::scale(translate(mat4(), pos), scale);
	updateMatrices();
}

void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Light source
	vec3 light(0.f, 0.f, 1.f);
	glUniform3fv(location["light"], 1, (GLfloat *)&light);
	vec3 viewer(0.f, 0.f, 1.f);
	glUniform3fv(location["viewer"], 1, (GLfloat *)&viewer);
	updateMatrices();
	glUniform1f(location["ambient"], 0.3f);
	glUniform1f(location["diffuse"], 1.f);
	glUniform1f(location["specular"], 1.f);
	updateModel(vec4(1.f), light, vec3(0.03f));
	sphere->bind();
	sphere->renderSolid();

	// Scene
	matrix.model = scale(mat4(), vec3(SCENE_SCALE, SCENE_SCALE, SCENE_SCALE));
	updateMatrices();

	//static const float scale = SCENE_SCALE / 4, offset = 0.35f;
	switch (status.scene) {
	case 0:
		// (A) Draw a wire-frame sphere by calculating all
		// the vertex positions and drawing lines between them.
		glUniform4f(location["colour"], 0.1f, 0.2f, 1.f, 1.f);
		sphere->renderFrame();
		break;
	case 1:
		// (B) For the sphere, work out the surface normal direction and
		// augment your wire-frame drawing with short lines representing
		// the normal direction of each vertex.
		// The sphere should now appear to be a hedgehog.
		glUniform4f(location["colour"], 0.1f, 0.2f, 1.f, 1.f);
		sphere->renderFrame();
		glUniform4f(location["colour"], 1.f, 0.f, 0.f, 1.f);
		sphere->renderNormal();
		break;
	case 2:
		// (C) Use the normal information calculated in (b) above work out
		// the amount of illumination falling on each vertex.
		// Assume that the light source is at infinity and is co-axial
		// with the viewpoint. Use this to draw a shaded sphere.
		updateModel(vec4(0.f, 0.f, 1.f, 1.f), vec3(0.f), vec3(0.8));
		sphere->renderSolid();
		break;
	case 3:
		// (D) Develop a simple animation showing a number of cones and spheres
		// moving along regular paths. These can be wireframe or solid.
		for (Model *model: models) {
			model->object->bind();
			for (Model::Position pos: model->position) {
				matrix.model = pos.model;
				updateMatrices();
				model->object->renderSolid();
			}
		}
		break;
	case 4:
		// (E) Draw a textured object, such as a rectangle (plane), box or sphere.
		glUniform4f(location["colour"], 0.4f, 0.8f, 1.f, 1.f);
		glUniform1f(location["specular"], 0.3f);
		sphere->renderSolid();
		break;
	case 5:
		updateModel(vec4(1.f, 0.f, 0.f, 1.f), vec3(0.f), vec3(SCENE_SCALE + 0.05f));
		sphere->renderFrame();
		updateModel(vec4(0.f, 1.f, 0.f, 1.f), vec3(0.f), vec3(SCENE_SCALE - 0.05f));
		sphere->renderSolid();
		updateModel(vec4(0.f, 0.f, 1.f, 0.7f), vec3(0.f), vec3(SCENE_SCALE));
		sphere->renderSolid();
		glUniform4f(location["colour"], 0.f, 1.f, 0.f, 1.f);
		sphere->renderNormal();
		break;
	}
}

void timerCB()
{
	for (Model *model: models)
		for (Model::Position &pos: model->position)
			pos.model = rotate<GLfloat>(mat4(), pi<GLfloat>() / 100.f, vec3(1.f, 1.f, 1.f)) * pos.model;
}

void refreshCB(GLFWwindow *window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	GLfloat asp = (GLfloat)height / (GLfloat)width;
	glViewport(0, 0, width, height);
	matrix.projection = ortho<GLfloat>(-1.f, 1.f, -asp, asp, -10, 10);
}

void keyCB(GLFWwindow */*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	switch (key) {
	case GLFW_KEY_ESCAPE:
	case GLFW_KEY_Q:
		glfwTerminate();
		exit(0);
		return;
	case GLFW_KEY_A ... (GLFW_KEY_A + SCENE_NUM - 1):
		status.scene = key - GLFW_KEY_A;
		return;
	case GLFW_KEY_SPACE:
		status.scene = (status.scene + 1) % SCENE_NUM;
		return;
	}

	vec3 rot;
	switch (key) {
	case GLFW_KEY_UP:
		rot = -vec3(1.f, 0.f, 0.f);
		break;
	case GLFW_KEY_DOWN:
		rot = vec3(1.f, 0.f, 0.f);
		break;
	case GLFW_KEY_LEFT:
		rot = -vec3(0.f, 1.f, 0.f);
		break;
	case GLFW_KEY_RIGHT:
		rot = vec3(0.f, 1.f, 0.f);
		break;
	case GLFW_KEY_PAGE_UP:
		rot = -vec3(0.f, 0.f, 1.f);
		break;
	case GLFW_KEY_PAGE_DOWN:
		rot = vec3(0.f, 0.f, 1.f);
		break;
	case GLFW_KEY_R:
		matrix.view = mat4x4();
	default:
		return;
	}
	matrix.view = rotate(mat4(), pi<GLfloat>() / 72.f, rot) * matrix.view;
}

void getUniforms(GLuint program, const char **uniforms)
{
	if (!program)
		return;
	while (*uniforms) {
		location[*uniforms] = glGetUniformLocation(program, *uniforms);
		uniforms++;
	}
}

int main(int /*argc*/, char */*argv*/[])
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow *window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	/* Setup render program */
	shader_t shaders[] = {
		{GL_VERTEX_SHADER, FILE_VERTEX_SHADER},
		{GL_FRAGMENT_SHADER, FILE_FRAGMENT_SHADER},
		{0, NULL}
	};
	GLuint program = setupProgramFromFiles(shaders);
	if (program == 0) {
		glfwTerminate();
		return -1;
	}
	glUseProgram(program);
	location["position"] = glGetAttribLocation(program, "position");
	location["normal"] = glGetAttribLocation(program, "normal");
	const char *uniforms[] = {
		"colour", "light", "viewer",
		"mvpMatrix", "normalMatrix",
		"ambient", "diffuse", "specular", 0
	};
	getUniforms(program, uniforms);

	setupVertices();
	status.scene = 0;
	matrix.view = rotate(mat4(), -pi<GLfloat>() / 4.f, vec3(1.f, 1.f, 0.f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	refreshCB(window);

	double prev = glfwGetTime();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		render();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		double now = glfwGetTime();
		if (now - prev > 0.1) {
			timerCB();
			prev += 0.1;
		}
		//glfwWaitEvents();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
