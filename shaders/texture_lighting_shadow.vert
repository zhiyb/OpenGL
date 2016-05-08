#version 330

in vec3 position, normal;
in vec2 tex;
uniform vec3 viewer;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 shadowMatrix;

out VS_FS_INTERFACE {
	vec4 shadow;
	vec3 world;
	vec3 normal;
	vec3 eye;
	vec2 tex;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vec4 world = modelMatrix * vec4(position, 1.0);
	vertex.normal = normalMatrix * normal;
	vertex.eye = viewer - vec3(world);
	vertex.shadow = shadowMatrix * world;
	vertex.world = vec3(world);
	vertex.tex = tex;
}
