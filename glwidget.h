#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtWidgets>
#include <QOpenGLFunctions>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	explicit GLWidget(QWidget *parent = 0);
	~GLWidget(void);
	QPixmap *pixmap(void);

signals:
	void changed(void);

public slots:

protected:
	void initializeGL(void);
	void resizeGL(int w, int h);
	void paintGL(void);
	void timerEvent(QTimerEvent *);
	void mousePressEvent(QMouseEvent *e) {lastPos = e->pos();}
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *) {update();}

private:
	GLuint loadShader(GLenum type, const QByteArray& context);
	GLuint loadShaderFile(GLenum type, const char *path);
	GLuint bindTexture(const QImage &img);
	GLuint bindTexture(const QString &path) {return bindTexture(QImage(path));}
	void renderContext(const GLboolean enableTexture);
	void renderColourCoding(void);
	void savePixelData(void);
	QColor readPixel(const QPoint position);
	QColor readPixel(const int x, const int y) {return readPixel(QPoint(x, y));}
	void makeCube(void);
	void makeDebugTexture(void);
	void normalizeAngle(GLfloat *angle);

	QPoint lastPos;

	struct {
		void apply(GLfloat x, GLfloat y, GLfloat z)
		{
			QMatrix4x4 p;
			p.rotate(x, 1.0, 0.0, 0.0);
			p.rotate(y, 0.0, 1.0, 0.0);
			p.rotate(z, 0.0, 0.0, 1.0);
			m = p * m;
		}
		QMatrix4x4 m;
	} rotate;
	unsigned char *pixelData;
	GLuint cubeProgram;
	GLuint texture, debugTexture;
	QMatrix4x4 projection;
	QVector<QVector2D> texCoords;
	QVector<QVector3D> vertices, colours;
	QColor centerColour;
	QPixmap pix;

	volatile int fpsCounter;
};

#endif // GLWIDGET_H
