#version 130

in vec3 position, normal;
in vec2 tex;
uniform vec3 viewer;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec3 vf_world;
out vec3 vf_normal;
out vec3 vf_eye;
out vec2 vf_tex;

void main(void)
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
	vf_normal = normalMatrix * normal;
	vf_world = vec3(modelMatrix * vec4(position, 1.0));
	vf_eye = viewer - vf_world;
	vf_tex = tex;
}
