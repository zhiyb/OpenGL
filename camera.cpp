#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "global.h"

#define CAMERA_MOVEMENT	0.2f
#define CAMERA_ROTATE	(2.f * PI / 180.f)
#define CAMERA_ELEV	(2.f * PI / 180.f)
#define CAMERA_ACCEL	0.2f

using namespace std;
using namespace glm;

Camera::Camera() : pos(CAMERA_INIT_POS), rot(quat()), speed(0.f)
{
	input.cursor = vec2(-1.f, -1.f);
	input.pressed = false;
	backup();
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
#if 1
	case GLFW_KEY_W:
		pos += forward() * CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_S:
		pos += forward() * -CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_D:
		pos += right() * CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_A:
		pos += right() * -CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_K:
		pos += upward() * CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_J:
		pos += upward() * -CAMERA_MOVEMENT;
		return;
	case GLFW_KEY_COMMA:
		rot = glm::rotate(rot, CAMERA_ROTATE, forward());
		return;
	case GLFW_KEY_PERIOD:
		rot = glm::rotate(rot, -CAMERA_ROTATE, forward());
		return;
#endif
	}
}

void Camera::mouseCB(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			input.pressed = true;
			//double xpos, ypos;
			//glfwGetCursorPos(window, &xpos, &ypos);
			//input.cursor = vec2(xpos, ypos);
		} else {
			input.pressed = false;
			input.cursor = vec2(-1.f, -1.f);
		}
	}
}

void Camera::cursorCB(double xpos, double ypos)
{
	if (!input.pressed)
		return;
	vec2 pos(xpos, ypos);
	if (input.cursor.x >= 0 && input.cursor.y >= 0) {
		vec2 move(vec2(1.f, -1.f) * (input.cursor - pos));
		vec3 axis(cross(vec3(move, 0.f), vec3(0.f, 0.f, 1.f)));
		rot = glm::rotate(rot, 1.f / 512.f * glm::distance(input.cursor, pos), axis);
	}
	input.cursor = pos;
}

void Camera::updateCB(float time)
{
	pos += forward() * speed * time;
}

void Camera::backup()
{
	bak.pos = pos;
	bak.rot = rot;
}

void Camera::restore()
{
	pos = bak.pos;
	rot = bak.rot;
	speed = 0;
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

void Camera::elevate(float angle)
{
	rot = glm::rotate(rot, angle, vec3(1.f, 0.f, 0.f));
}

void Camera::print()
{
	clog << "Camera @(" << pos.x << ", " << pos.y << ", " << pos.z << "), (";
	clog << rot.w << ", " << rot.x << ", " << rot.y << ", " << rot.z << ")" << endl;
}

glm::mat4 Camera::view() const
{
	return lookAt(pos, pos + forward(), upward());
}
