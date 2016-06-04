#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <random>
#include <functional>
#include <cstdlib>

#include "helper.h"
#include "global.h"

#include "square.h"

#define BIRD_UPWARD	(9.81f * 0.1f)
#define BIRD_ACCEL	(-9.81f * 20.f)
#define PIPE_SPACING	0.75f

using namespace std;
using namespace glm;

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);
std::uniform_real_distribution<double> distribution;

GLFWwindow *window;

Square *square;

program_t programs[PROGRAM_COUNT];
texture_t textures[TEXTURE_COUNT];

struct {
	double time, start;
} status;

struct {
	double pos;
	double speed;
} bird;

struct pipe_t {
	GLfloat pos;
};

vector<pipe_t> pipes;

void quit();

void setupObjects()
{
	square = new Square;
}

static void renderBackground()
{
	glUseProgram(programs[PROGRAM_BACKGROUND].id);
	uniformMap &uniforms = programs[PROGRAM_BACKGROUND].uniforms;
	mat4 matrix = mat4();
	glUniformMatrix4fv(uniforms[UNIFORM_MAT_MV], 1, GL_FALSE, (GLfloat *)&matrix);
	glUniform1f(uniforms[UNIFORM_TIME], status.time);
	square->bind();
	square->render();
}

static void renderBird()
{
	glUseProgram(programs[PROGRAM_TEXTURED_BASIC].id);
	uniformMap &uniforms = programs[PROGRAM_TEXTURED_BASIC].uniforms;
	mat4 matrix = translate(mat4(), vec3(-0.8f, bird.pos, 0.f));
	matrix = scale(matrix, vec3(0.1f, 0.1f, 1.f));
	glUniformMatrix4fv(uniforms[UNIFORM_MAT_MV], 1, GL_FALSE, (GLfloat *)&matrix);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BIRD].id);
	square->bind();
	square->render();
}

static void renderPipes()
{
	glUseProgram(programs[PROGRAM_TEXTURED_BASIC].id);
	uniformMap &uniforms = programs[PROGRAM_TEXTURED_BASIC].uniforms;
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_PIPE].id);

	mat4 size = scale(mat4(), vec3(1.f / 8.f, 1.f, 1.f));
	unsigned int i = 0;
	for (pipe_t pipe: pipes) {
		square->bind();

		double x = (double)(i++ * 2) - (status.time - status.start) + 4.f;
		x /= 2.f;
		double y = 1.f + PIPE_SPACING / 2.f;
		mat4 pos = translate(mat4(), vec3(0.f, -pipe.pos, 0.f));
		mat4 trans = translate(mat4(), vec3(x, y, 0.f));
		mat4 matrix = pos * trans * size;
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MV], 1, GL_FALSE, (GLfloat *)&matrix);
		square->render();

		y = -y;
		pos = translate(mat4(), vec3(0.f, -pipe.pos, 0.f));
		trans = translate(mat4(), vec3(x, y, 0.f));
		matrix = pos * trans * size;
		glUniformMatrix4fv(uniforms[UNIFORM_MAT_MV], 1, GL_FALSE, (GLfloat *)&matrix);
		square->render();
	}
}

static void render()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderBackground();
	renderBird();
	renderPipes();
}

static void update()
{
	double time = glfwGetTime();
	double prev = status.time - status.start;
	double diff = time - status.time;
	status.time = time;
	double current = status.time - status.start;

	bird.pos += diff * bird.speed;
	bird.speed += 0.5f * BIRD_ACCEL * diff * diff;

	double iPrev, iCurrent;
	modf(prev, iPrev);
	modf(current, iCurrent);
	if (iCurrent != iPrev && ((int)iCurrent % 2)) {
		pipe_t pipe;
		pipe.pos = (distribution(generator) - 0.5f) * (2.f - PIPE_SPACING);
		pipes.push_back(pipe);
	}
}

static void check()
{
	unsigned int i = 0;
	for (pipe_t pipe: pipes) {
	}
}

static void reset()
{
	status.start = glfwGetTime();
	bird.pos = 0;
	bird.speed = BIRD_UPWARD;
	pipes.clear();
}

static void refreshCB(GLFWwindow *window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
	//GLfloat invasp = (GLfloat)height / (GLfloat)width;
	//matrix.projection = ortho<GLfloat>(-1.f, 1.f, -invasp, invasp, -1, 1);
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
	case GLFW_KEY_R:
		reset();
		break;
	case GLFW_KEY_SPACE:
		bird.speed = BIRD_UPWARD;
		break;
	}
}

void mouseCB(GLFWwindow */*window*/, int button, int action, int /*mods*/)
{
}

void quit()
{
	delete square;
	glfwTerminate();
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

	glGetError();	// There is an initialisation error
	//glEnable(GL_DEPTH_TEST);
	//checkError("enabling depth test");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (setupPrograms() || setupTextures()) {
		glfwTerminate();
		return -1;
	}

	setupObjects();

	glfwSetWindowRefreshCallback(window, refreshCB);
	glfwSetKeyCallback(window, keyCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	refreshCB(window);

	srand(time(0));
	reset();
	float past = status.start = glfwGetTime();
	unsigned int count = 0;
	while (!glfwWindowShouldClose(window)) {
		check();
		render();
		update();

		// FPS counter
		count++;
		float now = glfwGetTime();
		if (now - past > 1.f) {
			float fps = (float)count / (now - past);
			count = 0;
			past = now;

			ostringstream ss;
			ss << fps << " FPS";
			glfwSetWindowTitle(window, ss.str().c_str());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	quit();
	return 0;
}
