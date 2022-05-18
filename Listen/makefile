
CFLAGS = -O3  -I/usr/include  -I/opt/X11/include -Wall -DGL_SILENCE_DEPRECATION -Wno-deprecated-declarations

CXXFLAGS = -O3  -I/usr/include  -I/opt/X11/include -Wall -DGL_SILENCE_DEPRECATION -Wno-deprecated-declarations

CFILES= Geom.c		Getpalette.c	Scene.c		Utilities.c
 
COBJS= Geom.o		Getpalette.o	Scene.o		Utilities.o 

CCFILES= BuildTriangles.cpp	glutMain.cpp
 
CCOBJS = BuildTriangles.o	glutMain.o 

fireball:  ${COBJS} ${CCOBJS}
	${CXX} ${CFLAGS} -o test.x ${COBJS} ${CCOBJS} -lm  -L/opt/X11/lib  -lX11 -lpthread  -lglut -lGL -lGLU -lGLEW 


