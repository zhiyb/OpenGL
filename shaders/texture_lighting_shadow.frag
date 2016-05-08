#version 330

uniform sampler2D sampler;
uniform sampler2DShadow shadowSampler;

// Material properties
struct material_t {
	float shininess;
	vec3 ambient, diffuse, specular, emission;
};
uniform material_t material;

// Lighting
const int maxLights = 5;
struct light_t {
	bool enabled;
	vec3 ambient;
	vec3 colour;
	vec3 position;
	float attenuation;
	int shadow;
};
uniform light_t lights[maxLights];

// Interfaces
in VS_FS_INTERFACE {
	vec4 shadow;
	vec3 world;
	vec3 normal;
	vec3 eye;
	vec2 tex;
} fragment;
out vec4 FragColor;

void main(void)
{
	vec4 tex = vec4(material.diffuse, 1.0) * texture(sampler, fragment.tex);
	float shadow = textureProj(shadowSampler, fragment.shadow);
	vec3 world = fragment.world;
	vec3 normal = normalize(fragment.normal);
	vec3 eyeDirection = normalize(fragment.eye);
	if (tex.a < 0.5)
		discard;
	else
		tex.a = 1.0;

	vec3 scattered = vec3(0.0), reflected = vec3(0.0);
	float emission = max(dot(eyeDirection, normal), 0.0);
	for (int i = 0; i < maxLights; i++) {
		if (!lights[i].enabled)
			continue;
		vec3 lightDirection = normalize(lights[i].position - world);
		float distance = length(lights[i].position - world);
		float attenuation = min(1.0 / (lights[i].attenuation * distance * distance), 1.0);
		//if (attenuation < 1.0 / 256.0)
		//	continue;

		vec3 colour = attenuation * lights[i].colour;
		if (lights[i].shadow != 0)
			colour *= shadow;

		float diffuse = max(dot(lightDirection, normal), 0.0);
		scattered += attenuation * lights[i].ambient * material.ambient;
		scattered += colour * material.diffuse * diffuse;

		//vec3 ref = 2.0 * dot(lightDirection, normal) * normal - light;
		vec3 ref = reflect(-lightDirection, normal);
		float specular = 0.0;
		if (diffuse != 0.0)
			specular = min(pow(max(dot(eyeDirection, ref), 0.0), material.shininess / 10.0), 1.0);
		reflected += colour * material.emission * emission;
		reflected += colour * material.specular * specular;
	}

	vec3 colour = scattered * tex.rgb + reflected;
	FragColor = vec4(min(colour, vec3(1.0)), tex.a);
}
