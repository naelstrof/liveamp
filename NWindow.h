#ifndef NAELSTROF_WINDOW
#define NAELSTROF_WINDOW

#include <iostream>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>
#include <sys/time.h>
#include <cstring>

class NWindow
{
private:
	Display *dpy;
	Window win;
	GLXContext glc;
	Atom wmDeleteMessage;
	int fps;
	float SleepStep;
	timespec WaitTime, RememberTime;
	timeval oldtime, newtime;
public:
	unsigned RealFPS;
	bool Valid;
	NWindow(unsigned int*, unsigned int*, const char*, bool, int, char**);
	int SwapBuffer();
	int Close();
	int CheckOpen();
	bool ChangedSize(unsigned int*, unsigned int*);
	int GetMouse(int*, int*);
	int CapFPS(unsigned int);
	unsigned int GetKey();
};

#endif
