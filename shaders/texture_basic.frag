#version 330

out vec4 FragColor;
uniform vec3 ambient;
uniform sampler2D sampler;

in VS_FS_INTERFACE {
	vec2 tex;
} fragment;

void main(void)
{
	vec4 tex = texture(sampler, fragment.tex);
	FragColor = vec4(ambient * tex.rgb, tex.a);
}
