//modified by: Christian Russell
//date: May 31
//purpose: Homework 1
//
//cs3350 Summer 2017 Homework-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <list>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GLUT/glut.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 15000
#define GRAVITY 0.1
#define WATER_FLOW 0.0025

#define NATURAL_FLOW_Y 25
#define NATURAL_FLOW_X 100
#define NATURAL_FLOW_AMOUNT 50

#define BOUNCE_PENALTY 3

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
	bool isCircle;
};

struct Particle {
	Shape s;
	Vec velocity;
};

class Game {
	public:
		std::list<Particle*> particles;
		std::list<Shape*> shapes;
		int n;
		Game(){
			n=0;
			//Create Shapes
			for(int i = 0; i < 5; i++){
				Shape *box = new Shape();
				box->width = 100;
				box->height = 20;
				box->center.x = 120 + (100 * i);
				box->center.y = 500 - (75 * i);
				box->isCircle = false;
				shapes.push_back(box);

			}
			Shape *circle = new Shape();
			circle->radius = 150;
			circle->isCircle = true;
			circle->center.x = 700;
			circle->center.y = 0;
			shapes.push_back(circle);
		}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
bool isInShape(float x, float y, float z=0, bool inclusive=true);
Shape *getContainingShape(float x, float y, float z=0, bool inclusive=true);
void makeParticle(Game *game, int x, int y, int amount=1);
void applyCircleRebound(Particle *p, Shape *s);
void writeText(Shape *s, std::string text);


//Other Globals
Game game;

int main(void) {
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		makeParticle(&game, NATURAL_FLOW_X, NATURAL_FLOW_Y, NATURAL_FLOW_AMOUNT);
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void) {
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Homework 1");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void) {
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}
#define rnd() (float)rand() / (float)RAND_MAX
void makeParticle(Game *game, int x, int y, int amount) {
	if (game->n >= MAX_PARTICLES - amount){
		amount = MAX_PARTICLES - game->n;
	}
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	for(int i = 0; i < amount; i++){
		//position of particle
		//Particle *p = &game->particle[game->n];
		Particle *p = new Particle();
		(game->particles).push_back(p);
		p->s.center.x = x;
		p->s.center.y = WINDOW_HEIGHT - y;
		p->velocity.y = rnd()*2 - 1.0;
		p->velocity.x = rnd()*2 - 1.0;
		game->n++;
	}
}

void check_mouse(XEvent *e, Game *game) {
	static int savex = 0;
	static int savey = 0;
	static int n = 0;
/*
	game.box.width = 100;
	game.box.height = 10;
	game.box.center.x = 120 + 5*65;
	game.box.center.y = 500 - 5*60;
*/
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
		makeParticle(game, e->xbutton.x, e->xbutton.y);
	}
}

int check_keys(XEvent *e, Game *game) {
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.
	}
	return 0;
}

void movement(Game *game) {

	if (game->n <= 0){
		return;
	}
	//for(int i = 0; i < game->n; i++){
	for(std::list<Particle*>::iterator it = game->particles.begin(); it != game->particles.end(); ){
		Particle *p = *it;
		//Natural Forces
		p->velocity.y -= GRAVITY;
		p->velocity.x += WATER_FLOW;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;

		//check for collision with shapes...
		Shape *s = getContainingShape(p->s.center.x, p->s.center.y);
		if(s != nullptr){
			if(s->isCircle){
				applyCircleRebound(p, s);
			}else{
				if(p->s.center.y > s->center.y){
	    				p->s.center.y = s->center.y + s->height;
					p->velocity.y *= -0.5;
				}
			}
		}
		//check for off-screen
		if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
			//std::cout << "off screen" << std::endl;
			//game->particle[i] = game->particle[(game->n)-1];
			it++;
			game->particles.remove(p);
			(game->n)--;
		}else{
			it++;
		}
	}
}

void render(Game *game) {
	float w, h, r;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	int boxPos = 0;
	std::string boxTitles[] = {"Requirements","Design","Coding","Testing","Maintenance"};
	for(std::list<Shape*>::iterator it = game->shapes.begin(); it != game->shapes.end(); it++){
		Shape *s = *it;
		if(s->isCircle){
			glColor3ub(90,140,90);
			glPushMatrix();
			glTranslatef(s->center.x, s->center.y, s->center.z);
			r = s->radius;
			//glBegin(GL_LINE_LOOP);
			glBegin(GL_TRIANGLE_FAN);
			//for(int i = 0; i < r; i++){
				for(int j = 0; j < 40; j++){
					//Take center, then find each point on the edge
					glVertex2f(cos(j * ((3.14*2)/40))*(r), sin(j * ((3.14*2)/40))*(r));
				}
			//}
			glEnd();
			glPopMatrix();
		}else{
			//Draw Box
			glColor3ub(90,140,90);
			glPushMatrix();
			glTranslatef(s->center.x, s->center.y, s->center.z);
			w = s->width;
			h = s->height;
			glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
			glEnd();
			glPopMatrix();
			//Draw Label
			for(int i = 0; i < boxTitles[i].size(); i++){
				writeText(*it, boxTitles[boxPos]);
			}
			boxPos++;
		}
	}
	//draw all particles here
	//for(int i = 0; i < game->n; i++){
	for(std::list<Particle*>::iterator it = game->particles.begin(); it != game->particles.end(); it++){
		Particle *p = *it;
	    	glPushMatrix();
		glColor3ub((rand() % 20) + 150,(rand() % 20) + 160,(rand() % 20) + 220);
		Vec *c = &(p->s.center);
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}

Shape* getContainingShape(float x, float y, float z, bool inclusive) {
	for(std::list<Shape*>::iterator it = game.shapes.begin(); it != game.shapes.end(); it++){
		if(!(*it)->isCircle) {
			//Square
			Shape shape = *(*it);
			float leftBound =  shape.center.x - (shape.width);
			float rightBound = shape.center.x + (shape.width);
			float upperBound = shape.center.y + (shape.height);
			float lowerBound = shape.center.y - (shape.height);
			if(inclusive) {
				//Borders are considered part of the shape
				if(x < leftBound) {
					continue;
				}else if(x > rightBound) {
					continue;
				}else if(y > upperBound) {
					continue;
				}else if(y < lowerBound) {
					continue;
				}else {
					return *it;
				}
			}else {
				//Borders are not considered part of the shape
				if(x <= leftBound) {
					continue;
				}else if(x >= rightBound) {
					continue;
				}else if(y >= upperBound) {
					continue;
				}else if(y <= lowerBound) {
					continue;
				}else {
					return *it;
				}
			}
		}else {
			//Circle
			Shape shape = *(*it);
			float radius = shape.radius;
			float length = sqrt(pow(x - shape.center.x, 2) + pow(y - shape.center.y, 2));
			if(inclusive){
				if(radius >= length){
					//std::cout << "In circle.";
					return *it;
				}
			}else{
				if(radius > length){
					return *it;
					//std::cout << "In circle.";
				}
			}
		}
	}
	return nullptr;
}

void applyCircleRebound(Particle *p, Shape *s){
	//std::cout << "Applying physics to circle. " << std::endl;
	//Triangle dimensions
	float triangleBase = fabs(s->center.x - p->s.center.x);
	float triangleHeight = fabs(s->center.y - p->s.center.y);
	float triangleDiameter = sqrt(pow(triangleBase, 2) + pow(triangleHeight, 2));
	//Triangle angle
	float sin = (triangleHeight / triangleDiameter);
	float degrees = 1.0 / sin;
	//Use angle from bottom of screen to calculate bounce
	if(p->s.center.x < s->center.x){
		if(degrees > 45.0){
			p->velocity.y *= -0.45;
			if(p->velocity.x > 0){
				p->velocity.x *= -0.45;
			}
		}else if(degrees < 45.0){
			p->velocity.y *= -0.50;
			if(p->velocity.x > 0){
				p->velocity.x *= -0.50;
			}
		}else{
			p->velocity.y *= -0.70;
			if(p->velocity.x > 0){
				p->velocity.x *= -0.30;
			}
		}

	}else{
		//landed on top right of circle
		p->velocity.y *= -0.45;
	}
	//Apply penalty to remove from circle
	float slope = (p->s.center.y - s->center.y)/(p->s.center.x - s->center.x);
	while(getContainingShape(p->s.center.x, p->s.center.y, 0, false) == s){
		if(p->s.center.x > s->center.x){
			//p is to the right of circle's center
			p->s.center.x += BOUNCE_PENALTY;
			p->s.center.y += (1 * (BOUNCE_PENALTY * slope)) > BOUNCE_PENALTY ? BOUNCE_PENALTY :1 * (BOUNCE_PENALTY * slope) ;
		}else{
			//p is to the left of circle's center
			p->s.center.x -= BOUNCE_PENALTY;
			p->s.center.y += (-1 * (BOUNCE_PENALTY * slope)) > BOUNCE_PENALTY ? BOUNCE_PENALTY :1 * (BOUNCE_PENALTY * slope) ;
		}
	}
}
void writeText(Shape *s, std::string text){
	glRasterPos2f(s->center.x, s->center.y);
	glColor3ub(255,255,0);
	for(int i = 0; i < text.length(); i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
	}
}
