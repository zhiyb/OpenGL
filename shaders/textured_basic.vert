#version 330

in vec3 position;
in vec2 tex;
uniform mat4 mvMatrix;

out VS_FS_INTERFACE {
	vec2 tex;
} vertex;

void main(void)
{
	gl_Position = mvMatrix * vec4(position, 1.0);
	vertex.tex = tex;
}
