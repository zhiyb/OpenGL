#version 330

uniform sampler2D sampler;

out vec4 fragColor;

in VS_FS_INTERFACE {
	vec2 tex;
} fragment;

void main(void)
{
	fragColor = texture(sampler, fragment.tex);
}
