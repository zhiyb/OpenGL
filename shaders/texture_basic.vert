#version 330

in vec3 position;
in vec2 tex;
uniform mat4 mvpMatrix;

out VS_FS_INTERFACE {
	vec2 tex;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.tex = tex;
}
