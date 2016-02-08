TEMPLATE = app
CONFIG += opengl
CONFIG += c++11
#CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS += -static

DEFINES += GLEW_STATIC

INCLUDEPATH += D:\Programs\misc\OpenGL\glm
#INCLUDEPATH += D:\Programs\misc\OpenGL\glut
INCLUDEPATH += D:\Programs\misc\OpenGL\glew-1.13.0\include
INCLUDEPATH += D:\Programs\misc\OpenGL\glfw-3.1.2.bin.WIN32\include
LIBS += -LD:\Programs\misc\OpenGL\glew-1.13.0\lib\Release\Win32 -lglew32s
#LIBS += -LD:\Programs\misc\OpenGL\glut -lglut32 -lglu32
LIBS += -LD:\Programs\misc\OpenGL\glfw-3.1.2.bin.WIN32\lib-mingw-w64 -lglfw3 -lgdi32
LIBS += -lopengl32

SOURCES += \
    main.cpp \
    helper.cpp \
    sphere.cpp \
    object.cpp

DISTFILES += \
    vertex.vert \
    basic.frag \
    light.frag \
    light.vert \
    light_frag.frag \
    light_frag.vert

HEADERS += \
    helper.h \
    sphere.h \
    global.h \
    object.h
