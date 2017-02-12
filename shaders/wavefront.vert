#version 130

in vec3 position, normal;
in vec2 tex;
uniform vec3 viewer;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 shadowMatrix;

out vec4 vf_shadow;
out vec3 vf_world;
out vec3 vf_normal;
out vec3 vf_eye;
out vec2 vf_tex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vec4 world = modelMatrix * vec4(position, 1.0);
	vf_normal = normalMatrix * normal;
	vf_eye = viewer - vec3(world);
	vf_shadow = shadowMatrix * world;
	vf_world = vec3(world);
	vf_tex = vec2(tex.x, 1.0 - tex.y);
}
