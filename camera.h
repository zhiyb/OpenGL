#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
	Camera();
	void keyCB(int key);
	void updateCB(float time);

	void backup();
	void restore();

	void rotate(float angle);
	void accelerate(float v);
	void elevate(float angle);

	void setPosition(const glm::vec3 pos) {this->pos = pos;}
	void setRotation(const glm::quat rot) {this->rot = rot;}
	void setSpeed(float speed) {this->speed = speed;}
	glm::vec3 position() const {return pos;}
	glm::quat rotation() const {return rot;}
	glm::vec3 direction() const {return rot * glm::vec3(0.f, 0.f, -1.f);}
	glm::vec3 upward() const {return rot * glm::vec3(0.f, 1.f, 0.f);}

	void print();

private:
	struct {
		glm::vec3 pos;
		glm::quat rot;
	} bak;
	glm::vec3 pos;
	glm::quat rot;
	float speed;
};

#endif // CAMERA_H
