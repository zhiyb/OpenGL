#version 130

uniform sampler2D sampler;

struct material_t {
	vec3 ambient;
};
uniform material_t material;

out vec4 FragColor;

in vec2 vf_tex;

void main(void)
{
	vec4 tex = texture(sampler, vf_tex);
	FragColor = vec4(material.ambient * tex.rgb, tex.a);
}
