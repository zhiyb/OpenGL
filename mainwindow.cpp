#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	//QWidget *w = new QWidget(this);
	//setCentralWidget(w);
	//QHBoxLayout *layout = new QHBoxLayout(w);
	//layout->addWidget(gl = new GLWidget);
	gl = new GLWidget(this);
	//gl->makeCurrent();
	//layout->addWidget(pixmap = new Pixmap);
	setCentralWidget(pixmap = new Pixmap);
	pixmap->setPixmap(gl->pixmap());
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	//gl->setAttribute(Qt::WA_TranslucentBackground);
	gl->setVisible(false);
	//gl->makeCurrent();
	//gl->updateGL();
	//gl->startTimer(10);
	connect(gl, SIGNAL(changed()), pixmap, SLOT(update()));
	resize(300, 300);
	//startTimer(20);
}

MainWindow::~MainWindow()
{
}

void MainWindow::timerEvent(QTimerEvent *)
{
	/*QPixmap pix = gl->pixmap();
	if (!pix.isNull())
		pixmap->setPixmap(pix);*/
}
