#version 330

out vec4 FragColor;
uniform vec3 ambient;
uniform sampler2D sampler;
uniform sampler2D cube[6];

in VERTEX {
	vec2 texCoord;
	//vec3 colour;
} vertex;

void main(void)
{
	//vec3 colour = ambient * vertex.colour;
	vec3 colour = ambient * texture(sampler, vertex.texCoord).rgb;
	//vec3 colour = ambient * max(texture(sampler, vertex.texCoord).rgb, vertex.colour);

	FragColor = vec4(colour, 1.0);
}
