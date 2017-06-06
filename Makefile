# cs335 lab1
# to compile your project, type make and press enter

all: lab1

lab1: lab1.cpp
	g++ lab1.cpp -Wall -lX11 -lGL -lGLU -lm -std=c++11 -o lab1

mac: lab1.cpp
	g++ lab1.cpp -Wall -lX11 -lGL -lGLU -lm -std=c++11 -o lab1 -stdlib=libc++ -I/usr/X11R6/include -L/usr/X11R6/lib -framework GLUT -framework OpenGL -framework Cocoa -Wno-deprecated
clean:
	rm -f lab1
	rm -f *.o

