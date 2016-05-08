#version 330

uniform sampler2D sampler;

struct material_t {
	vec3 ambient;
};
uniform material_t material;

out vec4 FragColor;

in VS_FS_INTERFACE {
	vec2 tex;
} fragment;

void main(void)
{
	vec4 tex = texture(sampler, fragment.tex);
	FragColor = vec4(material.ambient * tex.rgb, tex.a);
}
