#version 330

out vec4 FragColor;
uniform float ambient;
uniform sampler2D sampler;

in VERTEX {
	vec2 texCoord;
	vec3 colour;
} vertex;

void main(void)
{
	//vec3 colour = ambient * vertex.colour;
	vec3 colour = ambient * texture(sampler, vertex.texCoord).rgb + (1.0 - ambient) * vertex.colour;
	//vec3 colour = ambient * texture(sampler, vertex.texCoord).rgb + ambient * vertex.colour;

	FragColor = vec4(colour, 1.0);
}
