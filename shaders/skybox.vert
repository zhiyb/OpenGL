#version 330

in vec3 position;
in vec2 texCoord;
uniform mat4 mvpMatrix;

out VERTEX {
	vec2 texCoord;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.texCoord = texCoord;
}
