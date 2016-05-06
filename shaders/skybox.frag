#version 330

out vec4 FragColor;
uniform vec3 ambient;
uniform sampler2D sampler;
//uniform sampler2D cube[6];

in VERTEX {
	vec2 texCoord;
} vertex;

void main(void)
{
	vec4 tex = texture(sampler, vertex.texCoord);
	FragColor = vec4(ambient * tex.rgb, tex.a);
	//FragColor = vec4(vertex.texCoord, 1.0, 1.0);
}
