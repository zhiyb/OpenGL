#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "global.h"

#define CAMERA_POSITION	3.f
//#define CAMERA_MOVEMENT	0.5f

#define CAMERA_ROTATE	(2.f * PI / 180.f)
#define CAMERA_ACCEL	0.2f
#define CAMERA_ELEV	0.2f

using namespace std;
using namespace glm;

Camera::Camera()
{
#ifdef BULLET
	pos = vec3(0.f, 0.f, arena.scale + CAMERA_POSITION);
#else
	pos = vec3(0.f, 0.f, 1.f + CAMERA_POSITION);
#endif
	rot = quat();
}

void Camera::keyCB(int key)
{
	switch (key) {
	case GLFW_KEY_LEFT:
		// Turn camera to the left
		rotate(CAMERA_ROTATE);
		return;
	case GLFW_KEY_RIGHT:
		// Turn camera to the right
		rotate(-CAMERA_ROTATE);
		return;
	case GLFW_KEY_UP:
		// Increase the forward speed of the camera
		accelerate(CAMERA_ACCEL);
		return;
	case GLFW_KEY_DOWN:
		// Decrease the forward speed of the camera (minimum 0, stays)
		accelerate(-CAMERA_ACCEL);
		return;
	case GLFW_KEY_PAGE_UP:
		// Increase the elevation of the camera (optional)
		elevate(CAMERA_ELEV);
		return;
	case GLFW_KEY_PAGE_DOWN:
		// Decrease the elevation of the camera (optional)
		elevate(-CAMERA_ELEV);
		return;
	}
}

void Camera::updateCB(float time)
{
	pos += direction() * speed * time;
}

void Camera::rotate(float angle)
{
	rot = glm::rotate(rot, angle, vec3(0.f, 1.f, 0.f));
}

void Camera::accelerate(float v)
{
	speed += speed * v;
	if (v > 0) {
		if (speed < v)
			speed = v;
	} else {
		if (speed < -v)
			speed = 0.f;
	}
}

void Camera::elevate(float v)
{
	pos += vec3(0.f, v, 0.f);
}
