/*
 * Copyright (C) 2011 Dalton Nell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <time.h>
#include <GL/glew.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <signal.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glfw.h>
#include "NPulse.h"
#include "NShaderManager.h"
#include "NTexture.h"
#include <sys/stat.h>

#define PI 3.14159265

static GLuint ProgramID;
static GLuint ColorID;
static GLuint UniformID;
static GLuint WidthSize;
static GLuint HeightSize;
static GLuint UVBuffer;
static GLuint VertexBuffer;
static GLuint AmpID;

static unsigned int Width = 0;
static unsigned int Height = 0;
static unsigned int MaxFPS = 60;

float Verticies[] = {
	0,0,
	Width,0,
	Width,Height,
	Width,Height,
	0,Height,
	0,0};


float UVs[] = {
	0,0,
	1,0,
	1,1,
	1,1,
	0,1,
	0,0};

int DrawFullscreenQuad(NTexture::Texture texture, float Amp, float r, float g, float b)
{
	glUseProgram(ProgramID);
	
	glActiveTexture(GL_TEXTURE0);
	texture.Apply();
	glEnable(GL_TEXTURE_2D);
	glUniform1i(UniformID, 0);
	glUniform1f(WidthSize, Width);
	glUniform1f(HeightSize, Height);
	glUniform1f(AmpID, Amp);
	glUniform3f(ColorID,r,g,b);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,VertexBuffer);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);
	
	glDrawArrays(GL_TRIANGLES,0,6);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	return 0;
}

static int fps;
static float t1;
static timespec WaitTime, RememberTime;
Display *dpy;
Window win = 0;
GLXContext glc;

void sighandler(int signum)
{
	std::cout << "Recieved signal " << signum << ", exiting...\n";
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
    exit(0);
}

int main(int argc, char *argv[])
{
	//Check if operating files exist, if not copy them from default install {
	struct stat st;
	char buffer[128];
	char* confhome = getenv("XDG_CONFIG_HOME");
	strcpy(buffer,confhome);
	strcat(buffer,"/liveamp");
	if (stat(buffer,&st))
	{//ughgh this is ugly, but c string management sucks and i don't want to spend time to learn and import <string>.
		std::cout << buffer << " is missing, copying from base install.\n";
		char buffer2[128];
		char buffer3[128];
		strcpy(buffer2,"mkdir -p ");
		strcat(buffer2,buffer);
		system(buffer2);
		strcpy(buffer2,"cp -R");
		strcat(buffer2," /opt/liveamp/*s ");
		strcpy(buffer3,confhome);
		strcat(buffer3,"/liveamp");
		strcat(buffer2,buffer3);
		system(buffer2);
	}
	if (stat(buffer,&st))
	{
		std::cout << "Failed to create " << buffer << ", reverting to base install files.\n";
		chdir("/opt/liveamp");
	} else {
		chdir(buffer);
	}
	//}
	bool window = true;
	//Read and execute arguments {
	for (int i=0;i<argc;i++)
	{
		if (strcmp(argv[i],"-desktop") == 0 || strcmp(argv[i],"-d") == 0)
		{
			window = false;
		}
		if (strcmp(argv[i],"-w") == 0)
		{
			i++;
			if (i>argc)
			{
				std::cout << "Incorrect use of -w! Use --help for more info\n";
				return 0;
			}
			Width = atoi(argv[i]);
			if (Width<1)
			{
				std::cout << "Height is too low, clamping at 1...\n";
				MaxFPS = 1;
			}
		}
		if (strcmp(argv[i],"-h") == 0)
		{
			i++;
			if (i>=argc)
			{
				std::cout << "Incorrect use of -h! Use --help for more info\n";
				return 0;
			}
			Height = atoi(argv[i]);
			if (Height<1)
			{
				std::cout << "Height is too low, clamping at 1...\n";
				MaxFPS = 1;
			}
		}
		if (strcmp(argv[i],"-hz") == 0)
		{
			i++;
			if (i>=argc)
			{
				std::cout << "Incorrect use of -hz! Use --help for more info\n";
				return 0;
			}
			MaxFPS = atoi(argv[i]);
			if (MaxFPS<1)
			{
				std::cout << "hz is too low, clamping at 1...\n";
				MaxFPS = 1;
			}
		}
		if (strcmp(argv[i],"--h") == 0 || strcmp(argv[i],"--help") == 0)
		{
			std::cout << "liveamp Version: 1\n\nUsage: liveamp [-w WIDTH] [-h HEIGHT] [-hz REFRESHRATE] [-desktop] [-d] [--h] [--help]\n\n";
			std::cout << "Options:\n";
			std::cout << "\t-h\t\t- Window Height (-h 1920)\n";
			std::cout << "\t-w\t\t- Window Width (-h 1080)\n";
			std::cout << "\t-desktop\t- Attempt to run as a desktop wallpaper.\n";
			std::cout << "\t-d\t\t- Alias of -desktop.\n\n";
			std::cout << "\t-hz\t\t- Refresh rate of screen, defaults at 60hz. Faster refresh rates means less video-audio lag, but more image tearing.\n\n";
			std::cout << "Keybinds:\n";
			std::cout << "\tq\t\t- Quit\n\tESC\t\t- Quit\n\n";
			std::cout << "Notes:\n";
			std::cout << "\tYou can edit files in " << buffer << " to change the displayed picture or shaders (Look into shaders if you want to change the brightness or color transitions of the application!). ";
			std::cout << "There's also a way to make liveamp play a series of images as an animation, which an example file is included. ";
			std::cout << "If you mess up your configuration files, just delete the whole liveamp config folder and restart the program to get a fresh copy. ";
			std::cout << "\nMake sure to contact naelstrof@gmail.com to report bugs and request features!\n";
			return 0;
		}
	}
	//}
	std::cout << "Created by Naelstrof <naelstrof@gmail.com> with indirect help from David Reveman <davidr@novell.com>.\n";
	//Initwindow {
	if (!glfwInit()) //glfw for accurate timer, i'll try to remove it later as it's really bloated for just being a timer.
	{
		fprintf(stderr,"Failed to initialize GLFW!\n");
		return -1;
	}
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	XWMHints xwmh;

	dpy = XOpenDisplay(NULL);

	if ( !dpy ) {
		printf("Cannot connect to X server!\n");
		return 1;
	}
	Window root = DefaultRootWindow(dpy);
	XWindowAttributes gwa;
	XGetWindowAttributes(dpy, root, &gwa);
	if (Width == 0)
	{
		Width = gwa.width;
	}
	if (Height == 0)
	{
		Height = gwa.height;
	}
	vi = glXChooseVisual(dpy, 0, att);

	if (!vi) {
		printf("No appropriate visual found!\n");
		return 1;
	}
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask;
	
	win = XCreateWindow(dpy, root, 0, 0, Width, Height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	XMapWindow(dpy, win);
	if (!window) //If we're running as desktop's wallpaper, disable input, persist on all workspaces, and disable visibility on taskbars.
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
	XStoreName(dpy, win, "liveamp");
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!\n";
		return 1;
	}
	glClearColor(0.392156863,0.584313725,0.929411765,0.0);
	glEnable(GL_CULL_FACE);
	//}

	//Initgraphics {
	Verticies[2]=Width;
	Verticies[4]=Width;
	Verticies[6]=Width;
	Verticies[5]=Height;
	Verticies[7]=Height;
	Verticies[9]=Height;
	NTexture::Texture DesktopTexture("textures/desktop");
	if (!DesktopTexture.Valid)
	{
		std::cout << "Failed to load file textures/desktop! Make sure it exists!\n";
		glfwTerminate();
		return 1;
	}
	ProgramID = LoadShaders("shaders/flat.vert", "shaders/flat.frag");
	ColorID = glGetUniformLocation(ProgramID, "Color");
	AmpID = glGetUniformLocation(ProgramID, "Amp");
	UniformID = glGetUniformLocation(ProgramID, "TextureSampler");
	WidthSize = glGetUniformLocation(ProgramID, "ScreenWidth");
	HeightSize = glGetUniformLocation(ProgramID, "ScreenHeight");
	
	glGenBuffers(1,&VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 48, Verticies, GL_STATIC_DRAW);
	
	glGenBuffers(1,&UVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glBufferData(GL_ARRAY_BUFFER, 48, UVs, GL_STATIC_DRAW);
	
	//}
	
	//Initsoundsink { 
	NPulse Listener;
	WaitTime.tv_sec = 0;
	WaitTime.tv_nsec = 11000000;
	t1 = glfwGetTime();
	// }
	
	//loop {
	for (int i=1;i<=15;i++)
	{
		signal(i, sighandler);
	}
	float SleepStep = 0;
	bool Running = true;
	while(Running)
	{
		nanosleep(&WaitTime, &RememberTime);
		//Check events for window resize or keypresses, but only if we're not running as a desktop wallpaper {
		if (window)
		{
			XEvent xev;
			XCheckTypedEvent(dpy, KeyPress, &xev);
			if(xev.type == KeyPress) {
				if (xev.xkey.keycode == 9 || xev.xkey.keycode == 24) {
					std::cout << "Received keycode " << xev.xkey.keycode << ", exiting...\n";
					glXMakeCurrent(dpy, None, NULL);
					glXDestroyContext(dpy, glc);
					XDestroyWindow(dpy, win);
					XCloseDisplay(dpy);
					return 0;
				}
			}
			XCheckTypedEvent(dpy, Expose, &xev);
			if (xev.type == Expose)
			{
				XWindowAttributes gwa;
				XGetWindowAttributes(dpy, win, &gwa);
				Width = gwa.width;
				Height = gwa.height;
				Verticies[2]=Width;
				Verticies[4]=Width;
				Verticies[6]=Width;
				Verticies[5]=Height;
				Verticies[7]=Height;
				Verticies[9]=Height;
				glDeleteBuffers(1,&VertexBuffer);
				glGenBuffers(1,&VertexBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
				glBufferData(GL_ARRAY_BUFFER, 48, Verticies, GL_STATIC_DRAW);
				glViewport(0,0,Width,Height);
			}
		}
		//}
		//Draw screen {
		glClear(GL_COLOR_BUFFER_BIT); //only needed for transparent images.
		float Time = glfwGetTime();
		float RColor = fabs(sin((PI+Time)/PI));
		float GColor = fabs(sin(Time/PI));
		float BColor = fabs(sin((2*PI+Time)/PI));
		DrawFullscreenQuad(DesktopTexture,Listener.GetAmp(),RColor,GColor,BColor);
		glXSwapBuffers(dpy,win);
		//}
		//Cap fps at 60 {
		fps++;
		if (glfwGetTime()-t1>1.f)
		{
			float FrameTime = 1000.f/float(fps);
			SleepStep=(1000.f/MaxFPS-(FrameTime-SleepStep));
			if (SleepStep < 0)
			{
				SleepStep = 0;
			}
			WaitTime.tv_nsec = SleepStep*1000000L;
			fps = 0;
			t1 = glfwGetTime();
		}
		//}
	}
	//}
	return 0;
}