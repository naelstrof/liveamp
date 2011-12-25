#ifndef NAELSTROF_TEXTURE
#define NAELSTROF_TEXTURE
#include <png.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <fstream>
#include <cmath>

namespace NTexture
{
	static bool Verbose = false;

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
		float StartTime;
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
