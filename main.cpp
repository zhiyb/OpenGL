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

#define FILE_VERTEX_SHADER	"light_frag.vert"
#define FILE_FRAGMENT_SHADER	"light_frag.frag"

#define SCENE_NUM	6
#define SCENE_SCALE	0.8f

using namespace std;
using namespace glm;

vector<vec3> vertices, normals;
vector<GLuint> indices;

static map<string, GLint> location;
static struct {
	mat4 model, view, projection;
} matrix;

static struct {
	int scene;
} status;

static Sphere sphere(32);

void setupVertices()
{
	vertices.push_back(vec3(-0.5f, -0.5f, 0.0f));
	vertices.push_back(vec3(0.5f, -0.5f, 0.0f));
	vertices.push_back(vec3(0.5f, 0.5f, 0.0f));
	vertices.push_back(vec3(-0.5f, 0.5f, 0.0f));
	normals.push_back(vec3(-0.5f, -0.5f, 0.0f));
	normals.push_back(vec3(0.5f, -0.5f, 0.0f));
	normals.push_back(vec3(0.5f, 0.5f, 0.0f));
	normals.push_back(vec3(-0.5f, 0.5f, 0.0f));
	sphere.setup();
}

void updateMatrices()
{
	mat4 mvpMatrix = matrix.projection * matrix.view * matrix.model;
	glUniformMatrix4fv(location["mvpMatrix"], 1, GL_FALSE, (GLfloat *)&mvpMatrix);
	mat4 normalMatrix = transpose(inverse(matrix.view * matrix.model));
	glUniformMatrix4fv(location["normalMatrix"], 1, GL_FALSE, (GLfloat *)&normalMatrix);
	vec3 light(0.f, 0.f, 1.f);
	light = normalize(vec3(normalMatrix * vec4(light, 1.f)));
	glUniform3fv(location["light"], 1, (GLfloat *)&light);
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
	updateModel(vec4(1.f), light, vec3(0.03f));
	sphere.renderSolid();

	// Scene
	matrix.model = scale(mat4(), vec3(SCENE_SCALE, SCENE_SCALE, SCENE_SCALE));
	updateMatrices();

	static const float scale = SCENE_SCALE / 4, offset = 0.35f;
	switch (status.scene) {
	case 0:
		// (A) Draw a wire-frame sphere by calculating all
		// the vertex positions and drawing lines between them.
		glUniform4f(location["colour"], 0.1f, 0.2f, 1.f, 1.f);
		sphere.renderFrame();
		break;
	case 1:
		// (B) For the sphere, work out the surface normal direction and
		// augment your wire-frame drawing with short lines representing
		// the normal direction of each vertex.
		// The sphere should now appear to be a hedgehog.
		glUniform4f(location["colour"], 0.1f, 0.2f, 1.f, 1.f);
		sphere.renderFrame();
		glUniform4f(location["colour"], 1.f, 0.f, 0.f, 1.f);
		sphere.renderNormal();
		break;
	case 2:
		// (C) Use the normal information calculated in (b) above work out
		// the amount of illumination falling on each vertex.
		// Assume that the light source is at infinity and is co-axial
		// with the viewpoint. Use this to draw a shaded sphere.
		updateModel(vec4(0.f, 0.f, 1.f, 1.f), vec3(0.f), vec3(0.8));
		sphere.renderSolid();
		break;
	case 3:
		// (D) Develop a simple animation showing a number of cones and spheres
		// moving along regular paths. These can be wireframe or solid.
		updateModel(vec4(1.f, 1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(1.f, 0.f, 0.f, 1.f), vec3(offset, 0.f, 0.f), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(0.f, 1.f, 1.f, 1.f), vec3(-offset, 0.f, 0.f), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(0.f, 1.f, 0.f, 1.f), vec3(0.f, offset, 0.f), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(1.f, 0.f, 1.f, 1.f), vec3(0.f, -offset, 0.f), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(0.f, 0.f, 1.f, 1.f), vec3(0.f, 0.f, offset), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(1.f, 1.f, 0.f, 1.f), vec3(0.f, 0.f, -offset), vec3(scale));
		sphere.renderSolid();
		updateModel(vec4(1.f, 1.f, 1.f, 1.f), vec3(1.5f, 0.f, 0.f), vec3(scale));
		sphere.renderSolid();
		break;
	case 4:
		// (E) Draw a textured object, such as a rectangle (plane), box or sphere.
		sphere.renderSolid();
		break;
	case 5:
		updateModel(vec4(1.f, 0.f, 0.f, 1.f), vec3(0.f), vec3(SCENE_SCALE + 0.05f));
		sphere.renderFrame();
		updateModel(vec4(0.f, 1.f, 0.f, 1.f), vec3(0.f), vec3(SCENE_SCALE - 0.05f));
		sphere.renderSolid();
		updateModel(vec4(0.f, 0.f, 1.f, 0.7f), vec3(0.f), vec3(SCENE_SCALE));
		sphere.renderSolid();
		glUniform4f(location["colour"], 0.f, 1.f, 0.f, 1.f);
		sphere.renderNormal();
		break;
	}
#if 1
	glUniform4f(location["colour"], 1.0, 1.0, 0.0, 1.0);
	matrix.model = mat4();
	updateMatrices();
	glDrawArrays(GL_LINE_LOOP, 0, 4);
#endif
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
	location["colour"] = glGetUniformLocation(program, "colour");
	location["light"] = glGetUniformLocation(program, "light");
	location["viewer"] = glGetUniformLocation(program, "viewer");
	location["mvpMatrix"] = glGetUniformLocation(program, "mvpMatrix");
	location["normalMatrix"] = glGetUniformLocation(program, "normalMatrix");

	setupVertices();
	status.scene = 0;
	matrix.view = rotate(mat4(), -pi<GLfloat>() / 4.f, vec3(1.f, 1.f, 0.f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint bVertex;
	glGenBuffers(1, &bVertex);
	glBindBuffer(GL_ARRAY_BUFFER, bVertex);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location["position"]);
	glVertexAttribPointer(location["position"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint bNormal;
	glGenBuffers(1, &bNormal);
	glBindBuffer(GL_ARRAY_BUFFER, bNormal);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location["normal"]);
	glVertexAttribPointer(location["normal"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint bIndex;
	glGenBuffers(1, &bIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bIndex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	refreshCB(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		render();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwWaitEvents();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
