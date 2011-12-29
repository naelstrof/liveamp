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
	Buffer = new short[MAX_BUFFER];
	Connected = true;
}

NPulse::~NPulse()
{
	if (Connected)
	{
		delete[] Buffer;
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
	if (!Connected)
	{
		Reload(); //If the previous reload failed, try again! Keep trying till pulseaudio comes back online.
		return 0;
	}
	int error = 0;
	pa_simple_read(SoundSocket, Buffer, MAX_BUFFER, &error);
	BufferSize = MAX_BUFFER;
	if (error != 0)
	{
		std::cout << "Got an error: " << pa_strerror(error) << ".\n";
		std::cout << "Reloading sound socket...\n";
		Reload(); //Attempts to reload the socket, this is only for when pulseaudio crashes.
	}
	float Amp = 0;
	unsigned int i;
	for (i=0;i<BufferSize;i+=QUALITY)
	{
		//std::cout << stof(Buffer,i) << "\n";
		Amp += stof(Buffer,i);
	}
	Amp/=i;
	Amp*=150;
	return Amp;
}

float NPulse::stof(short* Buffer, unsigned int Finder)
{
	return float(abs(Buffer[Finder]))/(float)SHRT_MAX;
}

int NPulse::Reload()
{
	delete[] Buffer;
	pa_simple_free(SoundSocket);
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
		return 1;
	}
	std::cout << "Successfully connected to sound server.\n";
	Buffer = new short[MAX_BUFFER];
	Connected = true;
	return 0;
}