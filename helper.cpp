#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <GL/glew.h>
#include "helper.h"

using namespace std;

char *readFile(const char *path)
{
	ifstream is;
	is.open(path, ifstream::binary);
	if (!is)
		return 0;

	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);

	char *buf = new char[length + 1];
	is.read(buf, length);
	is.close();
	*(buf + length) = '\0';
	return buf;
}

GLuint setupShader(const GLenum type, const char *source)
{
	if (!source)
		return 0;

	GLuint sh = glCreateShader(type);
	if (!sh)
		return 0;
	glShaderSource(sh, 1, &source, NULL);
	glCompileShader(sh);

	GLint i;
	glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &i);
	if (i > 1) {
		cout << "Compiling shader:" << endl << source << endl;
		char log[i];
		glGetShaderInfoLog(sh, i, &i, log);
		cout.write(log, i);
	}

	glGetShaderiv(sh, GL_COMPILE_STATUS, &i);
	if (i == GL_TRUE)
		return sh;
	glDeleteShader(sh);
	return 0;
}

GLuint setupProgram(const GLuint *shaders)
{
	GLuint program = glCreateProgram();
	if (!program)
		return 0;
	while (*shaders != 0)
		glAttachShader(program, *shaders++);
	glLinkProgram(program);

	int i;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &i);
	if (i > 1) {
		cout << "Linking program:" << endl;
		char log[i];
		glGetProgramInfoLog(program, i, &i, log);
		cout.write(log, i);
	}

	glGetProgramiv(program, GL_LINK_STATUS, &i);
	if (i == GL_TRUE)
		return program;
	glDeleteProgram(program);
	return 0;
}

GLuint setupProgramFromFiles(const shader_t *shaders)
{
	vector<GLuint> s;
	GLuint program;
	while (shaders->type != 0) {
		GLuint sh = setupShaderFromFile(shaders->type, shaders->path);
		if (sh == 0)
			goto failed;
		s.push_back(sh);
		shaders++;
	}
	s.push_back(0);
	if ((program = setupProgram(s.data())) == 0)
		goto failed;
	return program;

failed:
	while (!s.empty()) {
		glDeleteShader(s.back());
		s.pop_back();
	}
	return 0;
}

GLuint setupShaderFromFile(GLenum type, const char *path)
{
	char *source = readFile(path);
	GLuint sh = setupShader(type, source);
	delete[] source;
	return sh;
}
