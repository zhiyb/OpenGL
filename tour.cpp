#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/gtx/polar_coordinates.hpp>
#include "helper.h"
#include "world.h"
#include "global.h"
#include "camera.h"
#include "bullet.h"
#include "tour.h"

using namespace std;
using namespace glm;

struct waypoint_t {
	bool centred;
	vec3 centre;
	string id;
	struct {
		vec3 pos;
		quat rot;
	} camera;
	float duration;
};
static vector<waypoint_t> waypoints;

static struct tour_t {
	double timeElapsed;
	unsigned int step;
} tour;

void loadTour()
{
	ifstream datafs(DATA_TOUR);
	if (!datafs) {
		cerr << "Cannot open tour data file " DATA_TOUR << endl;
		return;
	}
	string line;
	waypoint_t wp;
	while (getline(datafs, line)) {
		if (line.empty() || line.at(0) == '#')
			continue;
		istringstream ss(line);
		string type;
		ss >> type;
		if (type == "Waypoint") {
			if (!wp.id.empty())
				waypoints.push_back(wp);
			ss >> wp.id;
			wp.centred = false;
		} else if (type == "Camera")
			ss >> wp.camera.pos >> wp.camera.rot;
		else if (type == "Duration")
			ss >> wp.duration;
		else if (type == "Centre") {
			ss >> wp.centre;
			wp.centred = true;
		}
	}
	if (!wp.id.empty())
		waypoints.push_back(wp);
}

static void loadWaypoint(int i)
{
	waypoint_t &wp = waypoints[i];
	camera.setPosition(wp.camera.pos);
	camera.setRotation(wp.camera.rot);
}

void initTour()
{
	if (waypoints.size() == 0)
		return;
	tour.step = 0;
	status.pauseDuration = tour.timeElapsed = glfwGetTime();
	status.mode = status_t::TourMode;
	status.run = true;
	loadWaypoint(0);
}

void updateTour()
{
	double time = glfwGetTime();
	double diff = time - tour.timeElapsed;
	while (diff >= waypoints[tour.step].duration) {
		tour.timeElapsed = time;
		tour.step++;
		diff = time - tour.timeElapsed;
		if (tour.step + 1 == waypoints.size()) {
			quitTour();
			return;
		}
	}
	waypoint_t &wp = waypoints[tour.step];
	waypoint_t &wpn = waypoints[tour.step + 1];
	float ratio = diff / wp.duration;
	//clog << diff << ", " << ratio << ", " << wp.duration << endl;
	if (wp.centred)
		camera.setPosition(polarMix(wp.camera.pos, wpn.camera.pos, wp.centre, ratio));
	else
		camera.setPosition(mix(wp.camera.pos, wpn.camera.pos, ratio));
	camera.setRotation(mix(wp.camera.rot, wpn.camera.rot, ratio));
	//clog << camera.position() << camera.rotation() << endl;
}

void quitTour()
{
	status.mode = status_t::CameraMode;
}
