#include "NWindow.h"

//Some xlib black magic to open a window and bind opengl to it.
NWindow::NWindow(unsigned int* Width, unsigned int* Height, const char* Name, bool Desktop, int argc, char** argv)
{
	Valid = false;
	SleepStep = 0;
	WaitTime.tv_sec = 0;
	WaitTime.tv_nsec = 11000000;
	RememberTime.tv_sec = 0;
	RememberTime.tv_nsec = 0;
	gettimeofday(&oldtime, NULL);
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	XWMHints xwmh;

	dpy = XOpenDisplay(NULL);

	if ( !dpy ) {
		std::cout << "Cannot connect to X server!\n";
		return;
	}
	Window root = DefaultRootWindow(dpy);
	XWindowAttributes gwa;
	XGetWindowAttributes(dpy, root, &gwa);
	if (*Width == 0)
	{
		*Width = gwa.width;
	}
	if (*Height == 0)
	{
		*Height = gwa.height;
	}
	vi = glXChooseVisual(dpy, 0, att);

	if (!vi) {
		std::cout << "No appropriate visual found!\n";
		return;
	}
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_mask = StructureNotifyMask | KeyPressMask;
	
	win = XCreateWindow(dpy, root, 0, 0, *Width, *Height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	XMapWindow(dpy, win);
	wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);
	XClassHint* WindowHint = XAllocClassHint();
    WindowHint->res_name = new char[strlen(Name)];
    WindowHint->res_class = WindowHint->res_name;
    strcpy(WindowHint->res_name, Name);
    XSetClassHint(dpy,win,WindowHint);
	if (Desktop) //If we're running as desktop's wallpaper, disable input, persist on all workspaces, and disable visibility on taskbars.
	{
		std::cout << "Attempting to run as a desktop wallpaper.\n";
		xwmh.flags = InputHint;
		xwmh.input = 0;
		XSetWMProperties(dpy,win,NULL,NULL,argv,argc, NULL, &xwmh, NULL);
		Atom state[] = {XInternAtom(dpy, "_NET_WM_STATE_STICKY", 0),
		XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", 0)};
		XChangeProperty(dpy, win, XInternAtom (dpy, "_NET_WM_STATE", 0),
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *)state, 2);
		Region region;
		region = XCreateRegion();
		if (region)
		{
			XShapeCombineRegion (dpy, win, ShapeInput, 0, 0, region, ShapeSet);
			XDestroyRegion (region);
		} else {
			std::cout << "Couldn't create XRegion! Trying to continue anyway...\n";
		}
	}
	XStoreName(dpy, win, Name);
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
	glViewport(0,0,*Width,*Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, *Width, *Height, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	Valid = true;
}

int NWindow::SwapBuffer()
{
	if (Valid)
	{
		glXSwapBuffers(dpy,win);
	} else {
		return 1;
	}
	return 0;
}

int NWindow::Close()
{
	if (Valid)
	{
		glXDestroyContext(dpy, glc);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		Valid = false;
		return 0;
	}
	return 1;
}

bool NWindow::ChangedSize(unsigned int* Width, unsigned int* Height)
{
	XEvent xev;
	XEvent xevmem;
	for (unsigned int i=0;i<30;i++) //Usually the X server will send more notifications than the program can handle, this is a workaround.
	{
		XCheckTypedEvent(dpy, ConfigureNotify, &xev);
		if (xev.type == ConfigureNotify)
		{
			xevmem = xev;
			continue;
		} else {
			xev = xevmem;
			break;
		}
	}
	if (xev.type == ConfigureNotify)
	{
		XConfigureEvent xce = xev.xconfigure;
		if ((unsigned int)xce.width == *Width && (unsigned int)xce.height == *Height)
		{
			return false;
		}
		*Width = xce.width;
		*Height = xce.height;
		glViewport(0,0,*Width,*Height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, *Width, *Height, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		return true;
	}
	return false;
}

unsigned int NWindow::GetKey()
{
	XEvent xev;
	XCheckTypedEvent(dpy, KeyPress, &xev);
	if(xev.type == KeyPress) {
		return xev.xkey.keycode;
	}
	return 0;
}

int NWindow::CheckOpen()
{
	XEvent xev;
	while (true) //Usually the X server will send more notifications than the program can handle, this is a workaround.
	{
		XCheckTypedEvent(dpy, ClientMessage, &xev);
		if((int)xev.type == ClientMessage && xev.xclient.data.l[0] != (int)wmDeleteMessage)
		{
			continue;
		} else {
			break;
		}
	}
	if((int)xev.xclient.data.l[0] == (int)wmDeleteMessage)
	{
		Close();
		return 0;
	}
	return 1;
}

int NWindow::CapFPS(unsigned int MaxFPS)
{
	gettimeofday(&newtime, NULL);
	double ElapsedTime = (newtime.tv_sec - oldtime.tv_sec) * 1000.0;
	ElapsedTime += (newtime.tv_usec - oldtime.tv_usec) / 1000.0;
	nanosleep(&WaitTime, &RememberTime);
	fps++;
	if (ElapsedTime>1000.f)
	{
		float FrameTime = 1000.f/float(fps);
		SleepStep=(1000.f/MaxFPS-(FrameTime-SleepStep));
		if (SleepStep < 0)
		{
			SleepStep = 0;
		}
		WaitTime.tv_nsec = SleepStep*1000000L;
		RealFPS = fps;
		fps = 0;
		gettimeofday(&oldtime, NULL);
	}
	return 0;
}

int NWindow::GetMouse(int* X, int* Y)
{
	Window NothingImportant,NothingImportant_two;
	int NothingImportant_three,NothingImportant_four;
	unsigned int mask_return;
	XQueryPointer(dpy, win, &NothingImportant, &NothingImportant_two, &NothingImportant_three, &NothingImportant_four, X, Y, &mask_return);
	return 0;
}
