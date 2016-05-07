#ifndef BULLET_H
#define BULLET_H

#include <map>
#include "global.h"

#include <btBulletDynamicsCommon.h>

#define BULLET_GROUND	"Ground"
#define BULLET_CAMERA	"Camera"

extern struct bullet_data_t {
	glm::vec3 gravity;
	struct rigid_data_t {
		btScalar restitution, friction;
		btScalar lin_damping, ang_damping;
	};
	std::map<std::string, rigid_data_t> rigid;
} bullet;

void bulletInit();
void bulletCleanup();
void bulletUpdate(double diff);
void bulletAddRigidBody(btRigidBody *rigidBody);
void bulletAddRigidBody(btRigidBody *rigidBody, const char *name);

btVector3 bulletGetOrigin(btRigidBody *rigidBody);
glm::mat4 bulletGetMatrix(btRigidBody *rigidBody);

static inline glm::vec3 from_btVector3(const btVector3 &v) {return glm::vec3(v.x(), v.y(), v.z());}
static inline btVector3 to_btVector3(const glm::vec3 &v) {return btVector3(v.x, v.y, v.z);}
static inline glm::quat from_btQuaternion(const btQuaternion &q) {return glm::quat(q.x(), q.y(), q.z(), q.w());}

#endif // BULLET_H
