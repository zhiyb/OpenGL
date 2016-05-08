#version 330

in vec3 position, normal;
in vec2 tex;
uniform vec3 viewer;
uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out VS_FS_INTERFACE {
	vec3 world;
	vec3 normal;
	vec3 viewer;
	vec2 tex;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vertex.normal = normalize(normalMatrix * normal);
	vertex.world = vec3(modelMatrix * vec4(position, 1.0));
	vertex.viewer = normalize(viewer - vertex.world);
	vertex.tex = tex;
}
