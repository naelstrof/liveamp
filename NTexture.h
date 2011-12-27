#ifndef NAELSTROF_TEXTURE
#define NAELSTROF_TEXTURE
#include <png.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <cmath>
#include <sys/time.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

namespace NTexture
{
	static bool Verbose = true;

	static std::vector<char*> TextureNames;
	static std::vector<GLuint> TextureIDs;

	int CleanUp();

	class Animation 
	{
	private:
		unsigned int fps;
	public:
		char* Name;
		GLuint* Images;
		unsigned int ImageCount;
		int SetName(const char*);
		int AddImage(GLuint);
		int SetFPS(float);
		float GetFPS();
		~Animation();
	};
		
	class Texture
	{
	private:
		GLuint TextureID;
		unsigned int AnimationCount;
		unsigned int ID;
		unsigned int AnimationID;
		GLuint LoadPngImage(const char*);
		timeval StartTime;
	public:
		bool Static;
		bool Valid;
		Animation* Animations;
		char Name[128];
		Texture(const char*);
		Texture();
		int Apply();
		int Play(const char*);
		~Texture();
	};
	static std::vector<Texture> Textures;
}

#endif
