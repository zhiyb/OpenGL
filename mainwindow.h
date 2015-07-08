#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "glwidget.h"

class Pixmap : public QWidget
{
	Q_OBJECT
public:
	Pixmap(QWidget *parent = 0) : QWidget(parent), pix(0) {setMinimumSize(0, 0);}
	void setPixmap(QPixmap *pixmap) {pix = pixmap;/* update();*/}

protected:
	void paintEvent(QPaintEvent *)
	{
		if (pix == 0)
			return;
		QPainter painter(this);
		painter.drawPixmap(pix->rect(), *pix);
	}

private:
	QPixmap *pix;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	void timerEvent(QTimerEvent *);

private:
	GLWidget *gl;
	Pixmap *pixmap;
};

#endif // MAINWINDOW_H
