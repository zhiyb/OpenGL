#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/gtc/noise.hpp>
#include "helper.h"
#include "world.h"
#include "global.h"
#include "camera.h"
#include "bullet.h"
#include "tour.h"

using namespace std;
using namespace glm;

void animation(double time)
{
	time = time / 2.f;
	float i = 0.f;
	for (const pair<string, object_t> &objpair: objects) {
		const object_t &obj = objpair.second;
		if (!obj.rigidBody || !obj.animation)
			continue;
		float x = perlin(vec2(i + time, i + time + 2.f));
		float z = perlin(vec2(i + time + 3.f, i + time + 4.f));
		obj.rigidBody->activate(true);
		obj.rigidBody->applyImpulse(btVector3(x, 0.f, z) * 30.f,
					    btVector3(0.f, 0.f, -0.2f));
		i++;
	}
}
