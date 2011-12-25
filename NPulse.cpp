#include "NPulse.h"

NPulse::NPulse()
{
	PrintVersion();
	Connected = false;
	pa_sample_spec ss;

	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 2;
	ss.rate = 44100;

	SoundSocket = pa_simple_new(NULL,               // Use the default server.
					"liveamp",           // Our application's name.
					PA_STREAM_RECORD,
					NULL,               // Use the default device.
					"Detects current amplitude of playing sounds.",            // Description of our stream.
					&ss,                // Our sample format.
					NULL,               // Use default channel map
					NULL,               // Use default buffering attributes.
					NULL               // Ignore error code.
					);
	if (SoundSocket == NULL)
	{
		std::cout << "Couldn't connect to sound server!\n";
		std::cout << "Is pulseaudio installed and running?\n";
		return;
	}
	std::cout << "Successfully connected to sound server.\n";
	Connected = true;
}

NPulse::~NPulse()
{
	if (Connected)
	{
		pa_simple_free(SoundSocket);
	}
}

int NPulse::PrintVersion()
{
	std::cout << "Using pulseaudio API version " << PA_API_VERSION << ".\n";
	return 0;
}

float NPulse::GetAmp()
{
	int error = 0;
	short* Buffer = new short[MAX_BUFFER];
	pa_simple_read(SoundSocket, Buffer, MAX_BUFFER, &error);
	BufferSize = MAX_BUFFER;
	if (error != 0)
	{
		std::cout << "Got an error: " << pa_strerror(error) << ".\n";
		std::cout << "Trying to continue anyway...\n";
	}
	float Amp = 0;
	float Temp;
	unsigned int i;
	unsigned int ZeroCount = 0;
	for (i=0;i<BufferSize && ZeroCount < 3;i+=QUALITY)
	{
		Temp = stof(Buffer,i);
		if (Temp == 0)
		{
			ZeroCount++;
		} else {
			Amp += stof(Buffer,i);
		}
	}
	Amp/=i;
	Amp*=150;
	delete[] Buffer;
	return Amp;
}

float NPulse::stof(short* Buffer, unsigned int Finder)
{
	//short Amp;
	//memcpy(&Amp,&Buffer[Finder],1);
	//Amp<<=8;
	//memcpy(&Amp,&Buffer[Finder+1],1);
	//return (float)abs(Amp)/32767.f;
	return float(abs(Buffer[Finder]))/(float)SHRT_MAX;
}