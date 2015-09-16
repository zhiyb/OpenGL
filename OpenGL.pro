TEMPLATE = app
CONFIG += console c++11 opengl
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += D:\Programs\misc\glut
LIBS += -lopengl32 -lglut32 -lglu32
LIBS += -LD:\Programs\misc\glut

SOURCES += main.cpp
