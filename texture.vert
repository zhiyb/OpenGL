#version 400

in vec3 position, normal;
in vec2 texCoord;
uniform mat4 mvpMatrix, normalMatrix;

out VERTEX {
	vec3 normal;
	vec2 texCoord;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.normal = normalize(vec3(normalMatrix * vec4(normal, 0.0)));
	vertex.texCoord = texCoord;
}
