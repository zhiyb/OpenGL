#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

void render(void)
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor4f(1.f, 0.f, 0.f, 1.f);
	glVertex3f(-1.f, -1.f, 0.f);
	glColor4f(0.f, 1.f, 0.f, 1.f);
	glVertex3f(0.f, 0.732, 0.f);
	glColor4f(0.f, 0.f, 1.f, 1.f);
	glVertex3f(1.f, -1.f, 0.f);
	glEnd();

	glFlush();
}

int main(int argc, char *argv[])
{
	cout << "Hello, world!" << endl;
	glutInit(&argc, argv);
	glutCreateWindow("OpenGL");
	glutInitWindowSize(640, 480);
	glutDisplayFunc(render);
	glutMainLoop();
	return 0;
}
