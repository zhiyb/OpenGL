#version 330

in vec3 position;
uniform mat4 mvMatrix;
out vec2 pos;

void main(void)
{
	gl_Position = mvMatrix * vec4(position, 1.0);
	pos = position.xy;
}
