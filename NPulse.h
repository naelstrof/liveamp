#ifndef NAELSTROF_PULSE

#include <iostream>
#include <cstring>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <climits>
#define MAX_BUFFER 8192
#define QUALITY 32 //1 to MAX_BUFFER. best to worst quality level

class NPulse
{
public:
	NPulse();
	~NPulse();
	bool Connected;
	float GetAmp();
	int PrintVersion();
	short* Buffer;
	int Reload();
private:
	unsigned int BufferSize;
	pa_simple* SoundSocket;
	float stof(short*, unsigned int);
};

#endif