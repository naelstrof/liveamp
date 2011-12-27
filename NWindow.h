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

class NWindow
{
private:
	Display *dpy;
	Window win;
	GLXContext glc;
	Atom wmDeleteMessage;
public:
	bool Valid;
	NWindow(unsigned int*, unsigned int*, const char*, bool, int, char**);
	int SwapBuffer();
	int Close();
	int CheckClosed();
	bool ChangedSize(unsigned int*, unsigned int*);
	unsigned int GetKey();
};

#endif