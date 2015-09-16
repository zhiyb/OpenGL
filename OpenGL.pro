TEMPLATE = app
CONFIG += opengl
#CONFIG += c++11
#CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS += -static

INCLUDEPATH += D:\Programs\misc\glut
LIBS += -lopengl32 -lglut32 -lglu32
LIBS += -LD:\Programs\misc\glut

SOURCES += main.cpp
