COMP3214 Coursework 3
=====================
Topic:	Arena
Author:	Yubo Zhi (normanzyb@gmail.com)
GitHub:	https://github.com/zhiyb/OpenGL

=====================
Introduction:

Basically, a football stadium model was loaded.
Few character models was also loaded.
There are ground mesh around it, so you may expore outside the stadium.

BULLET physics engine was used to give realistic camera and models interaction that follow physical laws.

=====================
Build instructions:

Compile all cpp files, with following libraries:
glew, glfw3, opengl32,
BulletDynamics, BulletCollision, linearMath,

On windows, gdi32 is also needed.

The folder include/ need to be added to compiler include paths.

Run the program under the source directory, where shader and other data files locate.

=====================
Key bindings:

ESC/Q/q		Exit the program
T/t		Start the tour
E/e		Exit tour mode
R/r		Reset environment and camera
<SPACE>		Pause/unpause motions

Camera control (available outside tour mode):
P/p		The screenshot location
L/l		Alternative view point 1
O/o		Alternative view point 2, overhead
M/m		Return to last camera position
<LEFT>		Turn camera left
<RIGHT>		Turn camera right
<PAGE UP>	Increase camera elevation angle
<PAGE DOWN>	Decrease camera elevation angle
<UP>		Increase camera forward speed
<DOWN>		Decrease camera moving speed
Mouse drag	Rotate camera
Mouse scroll	Change camera movement step size

The camera is "mounted" on a sphere, managed by the physics engine.
When motions are paused, camera can move freely.

=====================
File list:

data/*.txt	Program settings and data
images/*	Texture images
models/*	Wavefront .obj models, materials and textures
shaders/*	OpenGL shader files
include/*	Header files
screenshot.jpg	Screenshot
config.lua	Coursework configuration

bullet.cpp	Bullet physics engine related
camera.cpp	Camera control related
helper.cpp	Some program helper functions
main.cpp	Main control loop and object rendering
tour.cpp	Tour mode related
world.cpp	Environment, status, etc.

object.cpp	Base class for objects
skybox.cpp	Skybox (enclosed cube)
ground.cpp	Ground mesh object
wavefront.cpp	Wavefront object (models)
circle.cpp	Circle object (the sun/moon)
sphere.cpp	Sphere object (point lights)
square.cpp	Square object (not used)
cube.cpp	Cube object (not used)

=====================
Credits:

See credits.txt.
