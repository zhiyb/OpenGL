#version 400

out vec4 colour_out;
uniform vec3 viewer;
uniform vec3 light;
uniform vec4 colour;

in VERTEX {
	vec3 normal;
} vertex;

void main(void)
{
	vec3 normal = vertex.normal;

	vec3 ambient = 0.2 * colour.rgb;
	vec3 colourA = vec3(ambient);

	float diffuse = max(dot(light, normal), 0.0);
	colourA += diffuse * colour.rgb;

	if (diffuse != 0.0) {
		vec3 reflect = 2.0 * dot(light, normal) * normal - light;
		float specular = pow(max(dot(viewer, reflect), 0), 30);
		colourA += max(specular, 0);
	}

	colour_out = vec4(min(colourA, vec3(1.0)), colour.a);
}
