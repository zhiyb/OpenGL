#version 400

out vec4 colour_out;
uniform float ambient, diffuse, specular;
uniform vec3 viewer;
uniform vec3 light;
uniform vec4 colour;

in VERTEX {
	vec3 normal;
} vertex;

void main(void)
{
	vec3 normal = vertex.normal;

	vec3 colourA = ambient * colour.rgb;

	float cos_theta = max(dot(light, normal), 0.0);
	colourA += diffuse * cos_theta * colour.rgb;

	if (cos_theta != 0.0) {
		vec3 reflect = 2.0 * dot(light, normal) * normal - light;
		colourA += specular * min(pow(max(dot(viewer, reflect), 0.0), 30), 1.0);
	}

	colour_out = vec4(min(colourA, vec3(1.0)), colour.a);
}
