#version 400

out vec4 FragColor;
uniform uint textured;
uniform float shininess;
uniform vec3 ambient, diffuse, specular;
uniform vec3 light;
uniform sampler2D sampler;

in VERTEX {
	vec3 normal;
	vec3 viewer;
	vec2 texCoord;
} vertex;

void main(void)
{
	vec3 normal = vertex.normal;

	vec3 colour = ambient;

	float cos_theta = max(dot(light, normal), 0.0);
	cos_theta = 1.f;
	if (textured != 0) {
		colour += texture(sampler, vertex.texCoord).rgb * diffuse * cos_theta;
	} else
		colour += diffuse * cos_theta;

	if (cos_theta != 0.0) {
		vec3 ref = 2.0 * dot(light, normal) * normal - light;
		//vec3 ref = reflect(-light, normal);
		colour += specular * min(pow(max(dot(vertex.viewer, ref), 0.0), shininess / 10.0), 1.0);
	}

	FragColor = vec4(min(colour, vec3(1.0)), 1.0);
}
