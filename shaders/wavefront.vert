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
	vec3 viewer;
	vec2 tex;
} vertex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vec4 world_pos = modelMatrix * vec4(position, 1.0);
	vertex.normal = normalize(normalMatrix * normal);
	vertex.viewer = normalize(viewer - vec3(world_pos));
	vertex.shadow = shadowMatrix * world_pos;
	vertex.tex = vec2(tex.x, 1.0 - tex.y);
}
