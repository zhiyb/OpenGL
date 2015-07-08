#include "glwidget.h"
#include <QtDebug>
#include <QMatrix4x4>

#if _WIN32
#include <dwmapi.h>
#endif

GLWidget::GLWidget(QWidget *parent) :
	QOpenGLWidget(parent)
{
	cubeProgram = 0;
	pixelData = 0;
	centerColour = Qt::white;
	fpsCounter = 0;

	//setAttribute(Qt::WA_AlwaysStackOnTop);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

#ifdef __linux__
	setAttribute(Qt::WA_TranslucentBackground);
#elif _WIN32
	MARGINS m;
	m.cxLeftWidth = -1;
	m.cxRightWidth = -1;
	m.cyBottomHeight = -1;
	m.cyTopHeight = -1;
	DwmExtendFrameIntoClientArea((HWND)winId(), &m);
#endif
	setWindowOpacity(0.75);

#if 1
	QSurfaceFormat fmt;
	fmt.setOptions(fmt.DeprecatedFunctions);
	fmt.setRenderableType(fmt.OpenGL);
	fmt.setVersion(4, 0);
	fmt.setDepthBufferSize(24);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setStencilBufferSize(8);
	fmt.setSamples(4);
	fmt.setSwapBehavior(fmt.DoubleBuffer);
	fmt.setProfile(fmt.CompatibilityProfile);
	fmt.setSwapInterval(0);
	setFormat(fmt);
	QSurfaceFormat::setDefaultFormat(fmt);
	qDebug() << fmt;
#endif
}

GLWidget::~GLWidget(void)
{
	delete [] pixelData;
}

void GLWidget::initializeGL(void)
{
	/*QSurfaceFormat fmt = context()->format();
	fmt.setSwapInterval(0);
	context()->setFormat(fmt);*/

	qDebug() << format();
	//qDebug() << context()->format();

	initializeOpenGLFunctions();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	makeDebugTexture();
	makeCube();
	vertices.append(QVector3D(0, 0, 0));
	vertices.append(QVector3D(10, 0, 0));
	vertices.append(QVector3D(0, 0, 0));
	vertices.append(QVector3D(0, 10, 0));
	vertices.append(QVector3D(0, 0, 0));
	vertices.append(QVector3D(0, 0, 10));

	GLuint vsh = loadShaderFile(GL_VERTEX_SHADER, "cubeVertex.vsh"), fsh = loadShaderFile(GL_FRAGMENT_SHADER, "cubeFragment.fsh");
	if (vsh == 0 || fsh == 0)
		return;
	cubeProgram = glCreateProgram();
	glAttachShader(cubeProgram, vsh);
	glAttachShader(cubeProgram, fsh);
	glBindAttribLocation(cubeProgram, 0, "vertex");
	glLinkProgram(cubeProgram);
	glUseProgram(cubeProgram);

	glEnableVertexAttribArray(glGetAttribLocation(cubeProgram, "vertex"));
	glEnableVertexAttribArray(glGetAttribLocation(cubeProgram, "texCoords"));
	glBindTexture(GL_TEXTURE_2D, texture);

	//startTimer(1);
}

void GLWidget::resizeGL(int w, int h)
{
	//delete [] pixelData;
	//pixelData = new unsigned char [w * h * 4];
	glViewport(0, 0, w, h);
	projection = QMatrix4x4();
	projection.perspective(45, (float)w / (float)h, 0.1, 1000.0);
	projection.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
}

QPixmap *GLWidget::pixmap(void)
{
	return &pix;
	//return renderPixmap(300, 300);
	/*if (pixelData == 0)
		return QPixmap();
	QImage image(pixelData, width(), height(), QImage::Format_RGBA8888);
	return QPixmap::fromImage(image.mirrored());*/
}

QColor GLWidget::readPixel(const QPoint position)
{
	unsigned char data[4];
	glReadPixels(position.x(), position.y(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	return QColor(data[0], data[1], data[2], data[3]);
}

void GLWidget::savePixelData(void)
{
	glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
}

void GLWidget::renderContext(const GLboolean enableTexture)
{
	static GLfloat colours[6][3] = {
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1},
		{0, 1, 1},
		{1, 0, 1},
		{1, 1, 0},
	};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projection"), 1, GL_FALSE, projection.constData());
	glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "rotate"), 1, GL_FALSE, rotate.m.constData());
	glVertexAttribPointer(glGetAttribLocation(cubeProgram, "vertex"), 3, GL_FLOAT, GL_TRUE, 0, vertices.constData());
	glVertexAttribPointer(glGetAttribLocation(cubeProgram, "texCoords"), 2, GL_FLOAT, GL_FALSE, 0, texCoords.constData());
	glUniform1i(glGetUniformLocation(cubeProgram, "useTexture"), enableTexture);

	glBindTexture(GL_TEXTURE_2D, texture);
	for (int i = 0; i < 6; i++) {
		glUniform4f(glGetUniformLocation(cubeProgram, "colour"), colours[i][0], colours[i][1], colours[i][2], 1.f);
		/*if (enableTexture == GL_TRUE) {
			if (centerColour.redF() == colours[i][0] && \
					centerColour.greenF() == colours[i][1] && \
					centerColour.blueF() == colours[i][2])
				glBindTexture(GL_TEXTURE_2D, texture);
			else
				glBindTexture(GL_TEXTURE_2D, debugTexture);
		}*/
		glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
	}
	glBindTexture(GL_TEXTURE_2D, debugTexture);
	for (int i = 0; i < 6; i++) {
		;
	}

	glUniform1i(glGetUniformLocation(cubeProgram, "useTexture"), GL_FALSE);
	for (int i = 0; i < 3; i++) {
		glUniform4f(glGetUniformLocation(cubeProgram, "colour"), colours[i][0], colours[i][1], colours[i][2], 1.f);
		glDrawArrays(GL_LINES, 4 * 6 + i * 2, 2);
	}
}

void GLWidget::renderColourCoding(void)
{
	renderContext(GL_FALSE);
	savePixelData();
	centerColour = readPixel(width() / 2, height() / 2);
}

void GLWidget::paintGL(void)
{
	//renderColourCoding();
	renderContext(GL_TRUE);
	glFinish();
	fpsCounter++;
	//update();
}

void GLWidget::timerEvent(QTimerEvent *)
{
	float angle = 5.f;
	rotate.apply(angle, angle, angle);
	//vertices[0].setX(vertices[0].x() + 0.001);
	update();
	static int second = 0;
	if (QTime::currentTime().second() != second) {
		int count = fpsCounter;
		fpsCounter -= count;
		qDebug(QString("FPS: %1").arg(count).toLocal8Bit());
		second = QTime::currentTime().second();
	}
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
	rotate.apply((float)(e->pos().y() - lastPos.y()) / 1.5, (float)(e->pos().x() - lastPos.x()) / 1.5, 0);
	lastPos = e->pos();
	update();
	QOpenGLWidget::mouseMoveEvent(e);
}

GLuint GLWidget::loadShader(GLenum type, const QByteArray& context)
{
	GLuint shader = glCreateShader(type);
	const char *p = context.constData();
	int length = context.length();
	glShaderSource(shader, 1, &p, &length);
	glCompileShader(shader);

	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE)
		return shader;

	int logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	char log[logLength];
	glGetShaderInfoLog(shader, logLength, &logLength, log);
	qWarning(log);
	glDeleteShader(shader);
	return 0;
}

GLuint GLWidget::loadShaderFile(GLenum type, const char *path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning(QString("Cannot open file %1").arg(path).toLocal8Bit());
		return 0;
	}
	QByteArray context = f.readAll();
	f.close();
	return loadShader(type, context);
}

GLuint GLWidget::bindTexture(const QImage &img)
{
	GLuint texture;
	QImage glTexture = img.mirrored();
#if 0
	QLabel *l = new QLabel;
	QPixmap pix;
	pix.convertFromImage(glTexture);
	l->setPixmap(pix);
	l->show();
#endif
	if (img.isNull()) {
		qWarning("Null texture image!");
		return 0;
	}
	makeCurrent();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glTexture.width(), glTexture.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, glTexture.bits());
	//qDebug() << texture;
	return texture;
}

void GLWidget::makeCube(void)
{
	const float coords[6][4][3] = {
		{{+0.5, +0.5, +0.5}, {+0.5, -0.5, +0.5}, {+0.5, -0.5, -0.5}, {+0.5, +0.5, -0.5}},
		{{-0.5, +0.5, -0.5}, {-0.5, +0.5, +0.5}, {+0.5, +0.5, +0.5}, {+0.5, +0.5, -0.5}},
		{{-0.5, +0.5, +0.5}, {-0.5, -0.5, +0.5}, {+0.5, -0.5, +0.5}, {+0.5, +0.5, +0.5}},
		{{-0.5, +0.5, -0.5}, {-0.5, -0.5, -0.5}, {-0.5, -0.5, +0.5}, {-0.5, +0.5, +0.5}},
		{{-0.5, -0.5, +0.5}, {-0.5, -0.5, -0.5}, {+0.5, -0.5, -0.5}, {+0.5, -0.5, +0.5}},
		{{+0.5, +0.5, -0.5}, {+0.5, -0.5, -0.5}, {-0.5, -0.5, -0.5}, {-0.5, +0.5, -0.5}},
	};
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 4; j++) {
			texCoords.append(QVector2D(j == 2 || j == 3, j == 0 || j == 3));
			vertices.append(QVector3D(coords[i][j][0], coords[i][j][1], coords[i][j][2]));
		}
	texture = bindTexture("image.jpg");
}

void GLWidget::makeDebugTexture(void)
{
	return;
	debugTexture = bindTexture("debug.png");
}

void GLWidget::normalizeAngle(GLfloat *angle)
{
	while (*angle >= 360)
		*angle -= 360;
	while (*angle <= -360)
		*angle += 360;
}
