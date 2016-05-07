#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "bullet.h"
#include "helper.h"
#include "world.h"

using namespace std;
using namespace glm;

struct bullet_data_t bullet;

// Bullet physics
static btBroadphaseInterface* broadphase;
static btDefaultCollisionConfiguration* collisionConfiguration;
static btCollisionDispatcher* dispatcher;
static btSequentialImpulseConstraintSolver* solver;
static btDiscreteDynamicsWorld* dynamicsWorld;

static vector<btRigidBody *> rigidBodys;

void bulletInit()
{
	ifstream datafs(DATA_BULLET);
	if (!datafs) {
		cerr << "Cannot open bullet physics description file " DATA_BULLET << endl;
		return;
	}
	string line;
	while (getline(datafs, line)) {
		if (line.empty() || line.at(0) == '#')
			continue;
		istringstream ss(line);
		string item;
		ss >> item;
		if (!ss)
			continue;
		if (item == "Gravity")
			ss >> bullet.gravity;
		else {
			bullet_data_t::rigid_data_t data;
			ss >> data.restitution >> data.friction;
			ss >> data.lin_damping >> data.ang_damping;
			bullet.rigid[item] = data;
		}
	}

	// Set up world
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(to_btVector3(bullet.gravity));
}

btVector3 bulletGetOrigin(btRigidBody *rigidBody)
{
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	return trans.getOrigin();
}

mat4 bulletGetMatrix(btRigidBody *rigidBody) {
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	mat4 matrix;
	trans.getOpenGLMatrix((btScalar *)&matrix);
	return matrix;
}

void bulletCleanup()
{
	for (btRigidBody *rigidBody: rigidBodys) {
		dynamicsWorld->removeRigidBody(rigidBody);
		delete rigidBody;
	}
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;
}

void bulletAddRigidBody(btRigidBody *rigidBody)
{
	dynamicsWorld->addRigidBody(rigidBody);
	rigidBodys.push_back(rigidBody);
}

void bulletAddRigidBody(btRigidBody *rigidBody, const char *name)
{
	bullet_data_t::rigid_data_t &data = bullet.rigid[name];
	rigidBody->setRestitution(data.restitution);
	rigidBody->setFriction(data.friction);
	rigidBody->setDamping(data.lin_damping, data.ang_damping);
	bulletAddRigidBody(rigidBody);
}

void bulletUpdate(double diff)
{
	dynamicsWorld->stepSimulation(diff, 120);
}
