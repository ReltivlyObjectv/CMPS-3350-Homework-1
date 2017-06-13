# cs335 hw1
# to compile your project, type make and press enter

all: hw1

hw1: hw1.cpp
	g++ hw1.cpp libggfonts.a -Wall -lX11 -lGL -lGLU -lm -std=c++11 -o hw1

clean:
	rm -f hw1
	rm -f *.o

