#version 330

in vec3 position, normal;
in vec2 texCoord;
uniform vec3 viewer;
uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out VERTEX {
	vec3 normal;
	vec3 viewer;
	vec2 texCoord;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.normal = normalize(normalMatrix * normal);
	vertex.viewer = normalize(viewer - vec3(modelMatrix * vec4(position, 0.0)));
	vertex.texCoord = vec2(texCoord.x, 1.0 - texCoord.y);
	//vertex.texCoord = texCoord;
}
