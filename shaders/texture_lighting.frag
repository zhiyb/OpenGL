#version 330

out vec4 FragColor;
uniform float shininess;
uniform vec3 ambient, diffuse, specular, emission;
uniform vec3 environment, lightPosition, lightIntensity;
uniform sampler2D sampler;

in VS_FS_INTERFACE {
	vec3 world;
	vec3 normal;
	vec3 viewer;
	vec2 tex;
} fragment;

void main(void)
{
	vec4 tex = vec4(diffuse, 1.0) * texture(sampler, fragment.tex);
	vec3 light = normalize(lightPosition - fragment.world);
	vec3 normal = fragment.normal;
	if (tex.a < 0.5)
		discard;
	else
		tex.a = 1.0;

	vec3 colour = tex.rgb * ambient * environment;

	float cos_theta = max(dot(light, normal), 0.0);
	colour += tex.rgb * cos_theta * lightIntensity;

	cos_theta = max(dot(fragment.viewer, normal), 0.0);
	colour += emission * cos_theta * lightIntensity;

	if (cos_theta != 0.0) {
		//vec3 ref = 2.0 * dot(light, normal) * normal - light;
		vec3 ref = reflect(-light, normal);
		colour += specular * min(pow(max(dot(fragment.viewer, ref), 0.0), shininess / 10.0), 1.0) * lightIntensity;
	}

	FragColor = vec4(min(colour, vec3(1.0)), tex.a);
}
