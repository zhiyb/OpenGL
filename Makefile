TRG	= OpenGL
SRC	= main.cpp
INCDIRS	= .

LIBS	+= -lglut -lGL

include Makefile.defs

.PHONY: run
run: $(TRG)
	./$(TRG)
