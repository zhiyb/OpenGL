#version 330

uniform float time;

in vec2 pos;
out vec4 fragColor;

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

#define PI		3.14159265358979323846
#define screenWidth	1

float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
	float unit = screenWidth/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

void main(void)
{
	vec3 colour[2];
	colour[0] = vec3(0.1, 0.3, 0.9);
	colour[1] = vec3(1., 1., 1.);
	float n = pow(pNoise(pos + vec2(time / 5., 0.), 5), 0.6);
	fragColor.rgb = mix(colour[0], colour[1], n);
	fragColor.a = 1.;
}
