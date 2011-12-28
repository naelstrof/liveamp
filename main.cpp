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
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>

#include "NPulse.h"
#include "NShaderManager.h"
#include "NTexture.h"
#include "NWindow.h"

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
static int fps;
static timespec WaitTime, RememberTime;
timeval oldtime, newtime, starttime;

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

void sighandler(int signum)
{
	std::cout << "Recieved signal " << signum << ", exiting...\n";
    exit(0);
}

int CheckFiles()
{
	//Check if operating files exist, if not copy them from default install
	struct stat st;
	char buffer[128];
	char buffer2[128];
	char buffer3[128];
	char* confhome = getenv("XDG_CONFIG_HOME");
	strcpy(buffer,confhome);
	strcat(buffer,"/liveamp");
	if (stat(buffer,&st))
	{//ughgh this is ugly, but c string management sucks and i don't want to spend time to learn and import <string>.
		std::cout << buffer << " is missing, copying from base install.\n";
		strcpy(buffer2,"mkdir -p ");
		strcat(buffer2,buffer);
		system(buffer2); //mkdir -p $XDG_CONFIG_HOME/liveamp
		strcpy(buffer2,"cp -R");
		strcat(buffer2," /opt/liveamp/*s ");
		strcpy(buffer3,confhome);
		strcat(buffer3,"/liveamp");
		strcat(buffer2,buffer3);
		system(buffer2); //cp -R /opt/liveamp/*s $XDG_CONFIG_HOME/liveamp
	}
	if (stat(buffer,&st)) //check to make sure it actually copied
	{
		std::cout << "Failed to create " << buffer << ", reverting to base install files.\n";
		chdir("/opt/liveamp");
		return 1;
	} else {
		chdir(buffer);
	}
	return 0;
}

int ReadArguments(int argc, char** argv, bool* Desktop, unsigned int* Width, unsigned int* Height, unsigned int* MaxFPS)
{
	//Read and execute arguments
	for (int i=0;i<argc;i++)
	{
		if (strcmp(argv[i],"-desktop") == 0 || strcmp(argv[i],"-d") == 0)
		{
			*Desktop = true;
		} else if (strcmp(argv[i],"-w") == 0)
		{
			i++;
			if (i>argc)
			{
				std::cout << "Incorrect use of -w! Use --help for more info\n";
				exit(0);
			}
			*Width = atoi(argv[i]);
			if (*Width<1)
			{
				std::cout << "Width is too low, clamping at 1...\n";
				*Width = 1;
			}
		} else if (strcmp(argv[i],"-h") == 0)
		{
			i++;
			if (i>=argc)
			{
				std::cout << "Incorrect use of -h! Use --help for more info\n";
				exit(0);
			}
			*Height = atoi(argv[i]);
			if (*Height<1)
			{
				std::cout << "Height is too low, clamping at 1...\n";
				*Height = 1;
			}
		} else if (strcmp(argv[i],"-hz") == 0)
		{
			i++;
			if (i>=argc)
			{
				std::cout << "Incorrect use of -hz! Use --help for more info\n";
				exit(0);
			}
			*MaxFPS = atoi(argv[i]);
			if (*MaxFPS<1)
			{
				std::cout << "hz is too low, clamping at 1...\n";
				*MaxFPS = 1;
			}
		} else if (strcmp(argv[i],"--h") == 0 || strcmp(argv[i],"--help") == 0)
		{
			char buffer[128];
			char* confhome = getenv("XDG_CONFIG_HOME");
			strcpy(buffer,confhome);
			strcat(buffer,"/liveamp");
			std::cout << "liveamp Version: 3\n\nUsage: liveamp [-w WIDTH] [-h HEIGHT] [-hz REFRESHRATE] [-desktop] [-d] [--h] [--help]\n\n";
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
			exit(0);
		} else if (strcmp(argv[i],"") && i > 0)
		{
			std::cout << "Unkown argument: " << argv[i] << "\n";
			std::cout << "Use --h or --help to learn how to use this program.\n";
			exit(0);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	for (int i=1;i<=15;i++) //intercept ALL signals and exit on all of them.
	{
		signal(i, sighandler);
	}
	std::cout << "Created by Naelstrof <naelstrof@gmail.com> with indirect help from David Reveman <davidr@novell.com>.\n";
	CheckFiles();
	bool Desktop = false;
	ReadArguments(argc,argv,&Desktop,&Width,&Height,&MaxFPS);
	//Initwindow {
	NWindow Win(&Width,&Height,"liveamp",Desktop,argc,argv);
	if (!Win.Valid)
	{
		std::cout << "Couldn't create X window! Not sure anything can be done about this...\n";
		return 1;
	}
	//}

	//Initgraphics {
	glClearColor(1,1,1,0.0);
	glEnable(GL_CULL_FACE);
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
	NTexture::Texture DesktopTexture("textures/desktop");
	if (!DesktopTexture.Valid)
	{
		char WorkingDirectory[128];
		getcwd(WorkingDirectory,128);
		std::cout << "Failed to load file " << WorkingDirectory << "textures/desktop! Try reinstalling or deleting the config folder!\n";
		return 1;
	}
	bool Result;
	ProgramID = LoadShaders("shaders/flat.vert", "shaders/flat.frag", &Result);
	if (!Result)
	{
		std::cout << "There was an error with one of your shader files! Check the errors above and fix them!\n";
		std::cout << "If you're not trying to edit the shader files, your drivers or graphics card may not support programmable pipeline!\n";
		std::cout << "Reverting to fixed function pipeline...\n";
		glViewport(0,0,Width,Height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, 1, 1, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
	} else {
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
	}
	
	//}
	
	//Initsoundsink { 
	NPulse Listener;
	// }
	
	//loop {
	//these are all used to cap the application's FPS {
	float SleepStep = 0;
	WaitTime.tv_sec = 0;
	WaitTime.tv_nsec = 11000000;
	gettimeofday(&oldtime, NULL);
	gettimeofday(&starttime, NULL);
	//}
	bool Running = true;
	while(Running)
	{
		gettimeofday(&newtime, NULL);
		double ElapsedTime = (newtime.tv_sec - oldtime.tv_sec) * 1000.0; //Get elapsed time
		ElapsedTime += (newtime.tv_usec - oldtime.tv_usec) / 1000.0;
		//Check events for window resize or keypresses, but only if we're not running as a desktop wallpaper {
		if (!Desktop)
		{
			//Exit if XWindow is closed
			if (!Win.Open())
			{
				std::cout << "X window was closed, exiting...\n";
				return 0;
			}
			//Close if q or esc is pressed with the X window in focus
			unsigned int Key = Win.GetKey();
			if (Key == 9 || Key == 24)
			{
				std::cout << "Received keycode " << Key << ", exiting...\n";
				Win.Close();
				return 0;
			}
			//Resize the window properly when resized
			if (Win.ChangedSize(&Width,&Height))
			{
				if (Result)
				{
					float Verticies[] = {
						0,0,
						Width,0,
						Width,Height,
						Width,Height,
						0,Height,
						0,0};
					glDeleteBuffers(1,&VertexBuffer);
					glGenBuffers(1,&VertexBuffer);
					glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
					glBufferData(GL_ARRAY_BUFFER, 48, Verticies, GL_STATIC_DRAW);
				}
				glViewport(0,0,Width,Height);
			}
		}
		//}
		//Draw screen {
		glClear(GL_COLOR_BUFFER_BIT); //only needed for transparent images.
		double Time = (newtime.tv_sec - starttime.tv_sec) * 1000.0; //Get elapsed time
		Time += (newtime.tv_usec - starttime.tv_usec) / 1000.0;
		Time /= 1000.f;
		float RColor = fabs(sin((PI+Time)/PI));
		float GColor = fabs(sin(Time/PI));
		float BColor = fabs(sin((2*PI+Time)/PI));
		float AMP = Listener.GetAmp();
		if (Result) //If we have a working programmable pipeline, use DrawFullscreenQuad. Otherwise use fixed function pipeline
		{
			DrawFullscreenQuad(DesktopTexture,AMP,RColor,GColor,BColor);
		} else {
			DesktopTexture.Apply();
			glBegin(GL_QUADS);
				glColor3f(0.4+AMP*RColor,0.4+AMP*GColor,0.4+AMP*BColor);
				glVertex2f(0,0);
				glTexCoord2d(1,0);
				glVertex2f(0,1);
				glTexCoord2d(0,0);
				glVertex2f(1,1);
				glTexCoord2d(0,1);
				glVertex2f(1,0);
				glTexCoord2d(1,1);
			glEnd();
		}
		Win.SwapBuffer();
		//}
		//Cap fps at 60 {
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
			fps = 0;
			gettimeofday(&oldtime, NULL);
		}
		//}
	}
	//}
	return 0;
}