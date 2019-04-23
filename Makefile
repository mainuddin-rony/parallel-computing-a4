# Makefile for shallowWaterMT

DEBUGOPT = -O3 # set to -g for debugging, or -O3 for optimized code

CFLAGS	= -Wall	-W -std=c99 ${DEBUGOPT} -Wno-unused-parameter -Wno-unused-variable -DGL_SILENCE_DEPRECATION

CPLUSFLAGS	= -Wall	-W ${DEBUGOPT} -Wno-unused-parameter -Wno-unused-variable -DGL_SILENCE_DEPRECATION

IFLAGS  = -I/usr/include -I/usr/include/GL -I/usr/X11R6/include -I/usr/X11R6/include/GL

GL_LIBS = -L/usr/lib -L/usr/X11R6/lib -lX11 -lGL -lGLU -lglut

LDFLAGS = -lpthread -lm



a4: mainTIFF.o Solver.o VisTIFF.o rgbtiff.o Vector3.o\
	 barrier.o waveFrontParSolver.o state_array.o timing.o
	gcc  mainTIFF.o  Solver.o VisTIFF.o rgbtiff.o Vector3.o\
	 barrier.o waveFrontParSolver.o state_array.o timing.o ${LDFLAGS} -o a4

shallow_waterGL: mainGL.o Solver.o  VisGL.o VisTIFF.o rgbtiff.o Vector3.o barrier.o
	g++  mainGL.o Solver.o VisGL.o VisTIFF.o rgbtiff.o Vector3.o barrier.o ${GL_LIBS} ${LDFLAGS} -o shallow_waterGL

.PHONY: all
all: shallow_waterTIFF shallow_waterGL


mainTIFF.o: mainTIFF.c timing.h waveFrontParSolver.h
	gcc ${CFLAGS} ${IFLAGS} -c mainTIFF.c
	
mainGL.o: mainGL.cpp
	g++ ${CPLUSFLAGS} ${IFLAGS} -c mainGL.cpp 
	
Vector3.o: Vector3.c Vector3.h
	gcc ${CFLAGS} -c Vector3.c 
	

Solver.o: Solver.c Solver.h
	gcc ${CFLAGS} -c Solver.c 
	
VisGL.o: VisGL.cpp VisGL.h
	g++ ${CPLUSFLAGS} ${IFLAGS} -c VisGL.cpp 

VisTIFF.o: VisTIFF.c VisTIFF.h
	gcc ${CFLAGS} ${IFLAGS} -c VisTIFF.c

rgbtiff.o: rgbtiff.c rgbtiff.h
	gcc ${CFLAGS} ${IFLAGS} -c rgbtiff.c

barrier.o: barrier.c barrier.h
	gcc ${CFLAGS} ${IFLAGS} -c barrier.c

state_array.o: state_array.c state_array.h
	gcc ${CFLAGS} ${IFLAGS} -c state_array.c

waveFrontParSolver.o: waveFrontParSolver.c waveFrontParSolver.h barrier.h state_array.h
	gcc ${CFLAGS} -c waveFrontParSolver.c 


.PHONY: clean
clean:
	rm -rf *.o a4 shallow_water*

