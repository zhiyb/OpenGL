#version 330

in vec3 position, normal;
in vec2 texCoord;
uniform mat4 mvpMatrix;

out VERTEX {
	vec2 texCoord;
	vec3 colour;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.texCoord = texCoord;
	vertex.colour = position.y > 0 ? vec3(0.2, 0.4, 1.0) : vec3(0.133, 0.694, 0.298);
	//vertex.colour = position.y > 0 ? vec3(0.0, 0.0, 0.0) : vec3(0.133, 0.694, 0.298);
}
