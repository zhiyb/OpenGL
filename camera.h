#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
	Camera();
	void keyCB(int key);
	void mouseCB(int button, int action);
	void scrollCB(double yoffset);
	void cursorCB(double xpos, double ypos);
	void updateCB(float time);

	void setup();
	void render();

	void backup();
	void restore();

	void rotate(float angle);
	void accelerate(float v);
	void elevate(float angle);

	void setPosition(const glm::vec3 pos) {this->pos = pos;}
	void setRotation(const glm::quat rot) {this->rot = rot;}
	void setSpeed(float speed) {this->speed = speed;}
	glm::vec3 &position() {return pos;}
	glm::vec3 position() const {return pos;}
	glm::quat rotation() const {return rot;}
	glm::vec3 right() const {return rot * glm::vec3(1.f, 0.f, 0.f);}
	glm::vec3 upward() const {return rot * glm::vec3(0.f, 1.f, 0.f);}
	glm::vec3 forward() const {return rot * glm::vec3(0.f, 0.f, -1.f);}
	glm::mat4 view() const;

	void print();

private:
	void updateCalc();

	struct {
		glm::vec3 pos;
		glm::quat rot;
	} bak;
	glm::vec3 pos;
	glm::quat rot;
	float speed, movement;

	class Sphere *sphere;

	struct {
		bool pressed;
		glm::vec2 cursor;
	} input;
};

extern Camera camera;

#endif // CAMERA_H
