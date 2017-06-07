# cs335 hw1
# to compile your project, type make and press enter

all: hw1

lab1: hw1.cpp
	g++ hw1.cpp -Wall -lX11 -lGL -lGLU -lm -std=c++11 -o hw1

mac: hw1.cpp
	g++ hw1.cpp -Wall -lX11 -lGL -lGLU -lm -std=c++11 -o hw1 -stdlib=libc++ -I/usr/X11R6/include -L/usr/X11R6/lib -framework GLUT -framework OpenGL -framework Cocoa -Wno-deprecated
clean:
	rm -f hw1
	rm -f *.o

