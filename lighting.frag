#version 400

out vec4 FragColor;
uniform float ambient, diffuse, specular, specularPower;
uniform vec3 light;
uniform vec4 colour;

in VERTEX {
	vec3 normal;
	vec3 viewer;
} vertex;

void main(void)
{
	vec3 normal = vertex.normal;

	vec3 colourA = vec3(0.f, 0.f, 0.f);
	colourA += ambient * colour.rgb;

	float cos_theta = max(dot(light, normal), 0.0);
	colourA += diffuse * cos_theta * colour.rgb;

	if (cos_theta != 0.0) {
		vec3 ref = 2.0 * dot(light, normal) * normal - light;
		//vec3 ref = reflect(-light, normal);
		colourA += specular * min(pow(max(dot(vertex.viewer, ref), 0.0), specularPower), 1.0);
	}

	FragColor = vec4(min(colourA, vec3(1.0)), colour.a);
}
