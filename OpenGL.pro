#-------------------------------------------------
#
# Project created by QtCreator 2014-09-16T14:08:33
#
#-------------------------------------------------

QT       += core gui
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenGL
TEMPLATE = app


SOURCES += main.cpp\
    glwidget.cpp

HEADERS  += \
    glwidget.h

OTHER_FILES += \
    cubeFragment.fsh \
    cubeVertex.vsh

win32: LIBS += -ldwmapi
