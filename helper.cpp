#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "helper.h"
#include "global.h"

//#define DEBUG_UNIFORMS

using namespace std;

string basename(string &path)
{
	string token;
	{
		istringstream iss(path);
		while (getline(iss, token, '/'))
			path = token;
	}
	{
		istringstream iss(path);
		while (getline(iss, token, '\\'))
			path = token;
	}
	while (path.size() != 0 && isspace(path.at(0)))
		path.erase(path.begin());
	while (path.size() != 0 && isspace(path.at(path.size() - 1)))
		path.erase(path.end() - 1);
	//clog << __func__ << ": " << path << endl << token << endl;
	return path;
}

istream &operator>>(istream &stream, glm::vec3 &vec)
{
	return stream >> vec.x >> vec.y >> vec.z;
}

istream &operator>>(istream &stream, glm::vec4 &vec)
{
	return stream >> vec.x >> vec.y >> vec.z >> vec.w;
}

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

bool checkError(const char *msg)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		cerr << "Error (0x" << hex << err << dec << ") " << msg << endl;
		return true;
	}
	return false;
}

GLuint setupShader(const GLenum type, const char *source)
{
	if (!source)
		return 0;

	GLuint sh = glCreateShader(type);
	if (!sh)
		return 0;
	glShaderSource(sh, 1, &source, NULL);
	checkError("uploading shader source");
	glCompileShader(sh);

	GLint i;
	glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &i);
	if (i > 1) {
		clog << "Compiling shader:" << endl << source << endl;
		char log[i];
		glGetShaderInfoLog(sh, i, &i, log);
		clog.write(log, i);
	}

	glGetShaderiv(sh, GL_COMPILE_STATUS, &i);
	if (i == GL_TRUE)
		return sh;
	glDeleteShader(sh);
	return 0;
}

GLuint setupProgram(GLuint program, const GLuint *shaders)
{
	while (*shaders != 0)
		glAttachShader(program, *shaders++);
	glLinkProgram(program);

	int i;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &i);
	if (i > 1) {
		clog << "Linking program:" << endl;
		char log[i];
		glGetProgramInfoLog(program, i, &i, log);
		clog.write(log, i);
	}

	glGetProgramiv(program, GL_LINK_STATUS, &i);
	if (i == GL_TRUE)
		return 0;
	glDeleteProgram(program);
	return 1;
}

GLuint setupProgramFromFiles(GLuint program, const shader_t *shaders)
{
	vector<GLuint> s;
	while (shaders->type != 0) {
		GLuint sh = setupShaderFromFile(shaders->type, shaders->path);
		if (sh == 0) {
			cerr << "Error setup shader " << shaders->path << endl;
			goto failed;
		}
		s.push_back(sh);
		shaders++;
	}
	s.push_back(0);
	if (setupProgram(program, s.data()) != 0) {
		goto failed;
		cerr << "Error setup program" << endl;
	}
	return 0;

failed:
	while (!s.empty()) {
		glDeleteShader(s.back());
		s.pop_back();
	}
	return 1;
}

GLuint setupShaderFromFile(GLenum type, const char *path)
{
	char *source = readFile(path);
	GLuint sh = setupShader(type, source);
	delete[] source;
	return sh;
}

static void setupUniforms(GLuint index)
{
	GLuint program = programs[index].id;
	if (!program)
		return;
	GLint cnt;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &cnt);
#ifdef DEBUG_UNIFORMS
	clog << __func__ << ": number of uniforms: " << cnt << endl;
#endif
	char name[32];
	uniformMap &uniforms = programs[index].uniforms;
	for (GLint idx = 0; idx < cnt; idx++) {
		GLsizei len;
		GLint size;
		GLenum type;
		glGetActiveUniform(program, idx, sizeof(name), &len, &size, &type, name);
		uniforms[name] = glGetUniformLocation(program, name);
#ifdef DEBUG_UNIFORMS
		clog << name << '@' << uniforms[name] << ' ';
#endif
	}
#ifdef DEBUG_UNIFORMS
	clog << endl;
#endif
}

GLuint setupPrograms()
{
	static const shader_t shaders[PROGRAM_COUNT][3] = {
		[PROGRAM_BASIC] = {
			{GL_VERTEX_SHADER, SHADER_PATH "basic.vert"},
			{GL_FRAGMENT_SHADER, SHADER_PATH "basic.frag"},
			{0, NULL}
		},
		[PROGRAM_LIGHTING] = {
			{GL_VERTEX_SHADER, SHADER_PATH "lighting.vert"},
			{GL_FRAGMENT_SHADER, SHADER_PATH "lighting.frag"},
			{0, NULL}
		},
		[PROGRAM_TEXTURE] = {
			{GL_VERTEX_SHADER, SHADER_PATH "texture.vert"},
			{GL_FRAGMENT_SHADER, SHADER_PATH "texture.frag"},
			{0, NULL}
		},
		[PROGRAM_WAVEFRONT] = {
			{GL_VERTEX_SHADER, SHADER_PATH "wavefront.vert"},
			{GL_FRAGMENT_SHADER, SHADER_PATH "wavefront.frag"},
			{0, NULL}
		},
		[PROGRAM_SKYBOX] = {
			{GL_VERTEX_SHADER, SHADER_PATH "skybox.vert"},
			{GL_FRAGMENT_SHADER, SHADER_PATH "skybox.frag"},
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
		checkError("binding attribuates");
		if (setupProgramFromFiles(program, shaders[idx]) != 0)
			return 2;
		setupUniforms(idx);
	}
	return 0;
}

GLuint setupTextures()
{
	const struct textureInfo_t {
		//GLenum type;
		const char *file;
	} textureInfo[TEXTURE_COUNT] = {
		[TEXTURE_WHITE]		= {0},
		[TEXTURE_SPHERE]	= {TEXTURE_PATH "earth.png"},
		//[TEXTURE_FIREMAP]		= {TEXTURE_PATH "firemap.png"},
		// glow1.png: http://vterrain.org/Atmosphere/
		[TEXTURE_GLOW]		= {TEXTURE_PATH "glow1.png"},
		// diamond_block.png: Minecraft
		[TEXTURE_CUBE]		= {TEXTURE_PATH "diamond_block.png"},
		// skybox3.png: http://scmapdb.com/skybox:sky-blu
		[TEXTURE_SKYBOX]	= {TEXTURE_PATH "skybox3.png"},
		[TEXTURE_DEBUG]		= {TEXTURE_PATH "debug.png"},
	};

	//glActiveTexture(GL_TEXTURE0);
	for (GLuint i = 0; i < TEXTURE_COUNT; i++)
		if (textureInfo[i].file != 0)
			textures[i] = loadTexture(textureInfo[i].file);
		else {	// White texture
			texture_t &tex = textures[i];
			tex.x = 1;
			tex.y = 1;
#ifdef TEXTURE_ALPHA
			tex.n = 4;
#else
			tex.n = 3;
#endif
			glGenTextures(1, &tex.texture);
			if (tex.texture == 0) {
				cerr << "Cannot generate texture storage for white texture" << endl;
				continue;
			}
			glBindTexture(GL_TEXTURE_2D, tex.texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			const unsigned char data[] = {0xff, 0xff, 0xff, 0xff};
#ifdef TEXTURE_ALPHA
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.x, tex.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.x, tex.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
#endif
			if (checkError("uploading white texture")) {
				glDeleteTextures(1, &tex.texture);
				tex.texture = 0;
			}
		}

	return 0;
}

texture_t loadTexture(const char *path)
{
#ifdef TEXTURE_ALPHA
	const int channels = 4;
#else
	const int channels = 3;
#endif
	texture_t tex;
	tex.texture = 0;
	unsigned char *data = stbi_load(path, &tex.x, &tex.y, &tex.n, channels);
	if (data == 0) {
		cerr << "Error loading texture file " << path << endl;
		return tex;
	}
	if (tex.n != channels) {
		cerr << "Invalid image format from texture file " << path << endl;
		stbi_image_free(data);
		return tex;
	}
#ifdef WAVEFRONT_DEBUG
	clog << __func__ << ": Texture file " << path << " loaded, " << tex.x << "x" << tex.y << "-" << tex.n << endl;
#endif
	glGenTextures(1, &tex.texture);
	if (tex.texture == 0) {
		cerr << "Cannot generate texture storage for " << path << endl;
		stbi_image_free(data);
		return tex;
	}
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef TEXTURE_ALPHA
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.x, tex.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.x, tex.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
#endif
	stbi_image_free(data);
	if (checkError((string("uploading texture from ") + path).c_str())) {
		glDeleteTextures(1, &tex.texture);
		tex.texture = 0;
	}
	return tex;
}
