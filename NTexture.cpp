#include "NTexture.h"

GLuint NTexture::Texture::LoadPngImage(const char* name) //Shamelessly stolen from a random website explaining how to use libpng
{
	//Check to make sure png isn't already loaded
	for (unsigned int i=0;i<TextureNames.size();i++)
	{
		if (!strcmp(TextureNames[i],name))
		{
			return TextureIDs[i];
		}
	}
    //Prepare to load png
    FILE* fp;
    fp = fopen(name,"r");
    if (!fp)
    {
        std::cout << "Couldn't open " << name << "!\n";
        return -1;
    }
    unsigned char header[4];
    fread(header,1,4,fp);
    if (png_sig_cmp(header,0,4))
    {
        fclose(fp);
        std::cout << "Tried to load " << name << " as a PNG! (It's not a PNG!)\n";
        return -1;
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(fp);
        std::cout << "Couldn't create read structure for file " << name << "!\n";
        return -1;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(fp);
        png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
        std::cout << "Couldn't create info structure for file " << name << "!\n";
        return -1;
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
        fclose(fp);
        std::cout << "LIBPNG encountered an error and couldn't load " << name << "!\n";
        return -1;
    }
    png_init_io(png_ptr,fp);
    png_set_sig_bytes(png_ptr,4);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
    int Width = png_get_image_width(png_ptr, info_ptr);
    int Height =  png_get_image_height(png_ptr, info_ptr);
    bool HasAlpha;
    switch (png_get_color_type(png_ptr, info_ptr)) {
        case PNG_COLOR_TYPE_RGBA:
            HasAlpha = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            HasAlpha = false;
            break;
        default:
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            std::cout << name << " has an unknown color type: " << png_get_color_type(png_ptr, info_ptr) << ". It's not supported!\n";
            return -1;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    GLubyte* textureImage = new unsigned char[row_bytes * Height];

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < Height; i++) {
        memcpy(textureImage+(row_bytes * (Height-1-i)), row_pointers[i], row_bytes);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    fclose(fp);
    if (Verbose) {
		std::cout << "Successfully loaded " << name << ".\n";
	}
   
    GLuint texture;
    
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexImage2D(GL_TEXTURE_2D, 0, HasAlpha ? 4 : 3, Width, Height, 0, HasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    char* myName = (char*)malloc(strlen(name));
	strcpy(myName,name);
	TextureNames.push_back(myName);
	TextureIDs.push_back(texture);
	delete[] textureImage;
    return texture;
}

int NTexture::Animation::SetName(const char* NewName)
{
	Name = (char*)malloc(strlen(NewName));
	strcpy(Name,NewName);
	return 0;
}

int NTexture::Animation::AddImage(GLuint ImageID)
{
	Images[ImageCount] = ImageID;
	ImageCount++;
	return 0;
}

int NTexture::Animation::SetFPS(float NewFPS)
{
	fps = NewFPS;
	return 0;
}

float NTexture::Animation::GetFPS()
{
	return fps;
}

NTexture::Animation::~Animation()
{
	free(Name);
}

NTexture::Texture::Texture()
{
	//do nothing, incorrectly initialized
}

NTexture::Texture::Texture(const char* name)
{
	Valid = true;
	gettimeofday(&StartTime, NULL);
	AnimationID = 0;
	for (unsigned int i=0;i<Textures.size();i++)
	{
		if (!strcmp(name,Textures[i].Name))
		{
			Static = Textures[i].Static;
			if (Static)
			{
				TextureID = Textures[i].TextureID;
			} else {
				Animations = Textures[i].Animations;
				AnimationCount = Textures[i].AnimationCount;
				Play("idle");
			}
			return;
		}
	}
	strcpy(Name,name);
	AnimationCount=0;
	std::ifstream infile;
	infile.open(name,std::ifstream::in);
	if (!infile.good())
	{
		std::cout << "Could not open file: " << name << "\n";
		Valid = false;
		return;
	}
	char buffer[256];
	std::vector<std::string> Lines;
	while (infile.good())
	{
		infile.getline(buffer,256);
		Lines.push_back(std::string(buffer,256));
	}
	infile.close();
	for (unsigned int i=0;i<Lines.size();i++)
	{
		if (Lines[i][0] == 'a')
		{
			AnimationCount++;
		}
	}
	char buffer2[128];
	Animations = (Animation*)malloc(sizeof(Animation)*AnimationCount);
	float fps;
	unsigned int CurrentAnimation=0;
	for (unsigned int i=0;i<Lines.size();i++)
	{
		if (Lines[i][0] == 'a')
		{
			sscanf(Lines[i].data(),"anim %s %ffps {",buffer2,&fps);
			Animations[CurrentAnimation].SetFPS(fps);
			Animations[CurrentAnimation].SetName(buffer2);
			Animations[CurrentAnimation].Images = (GLuint*)malloc(sizeof(GLuint)*32);
			Animations[CurrentAnimation].ImageCount = 0;
			continue;
		}
		if (Lines[i][0] == '\t')
		{
			sscanf(Lines[i].data(),"\t%s",buffer2);
			Animations[CurrentAnimation].AddImage(LoadPngImage(buffer2));
			continue;
		}
		if (Lines[i][0] == '}')
		{
			CurrentAnimation++;
			continue;
		}
		if (Lines[i][0] == 's')
		{
			sscanf(Lines[i].data(),"static %s",buffer2);
			TextureID = LoadPngImage(buffer2);
			ID = Textures.size();
			Static = true;
			Textures.push_back(*this);
			Lines.clear();
			if (Verbose) {
				std::cout << "Loaded texture " << name << " as a static image!\n";
			}
			return;
		}
	}
	Lines.clear();
	ID = Textures.size();
	Static = false;
	Textures.push_back(*this);
	if (Verbose) {
		std::cout << "Loaded texture " << name << " with " << AnimationCount << " detected animations!\n";
	}
	Play("idle");
}

int NTexture::Texture::Play(const char* name)
{
	if (Static)
	{
		return 1;
	}
	for (unsigned int i=0;i<AnimationCount;i++)
	{
		if (!strcmp(name,Animations[i].Name))
		{
			AnimationID = i;
			gettimeofday(&StartTime, NULL);
			return 0;
		}
	}
	std::cout << "Couldn't find animation " << name << " in a texture!\n";
	return 1;
}

int NTexture::Texture::Apply()
{
	int Frame=0;
	if (Static)
	{
		glBindTexture(GL_TEXTURE_2D, TextureID);
	} else {
		timeval newtime;
		gettimeofday(&newtime, NULL);
		double ElapsedTime = (newtime.tv_sec - StartTime.tv_sec) * 1000.0;
		ElapsedTime += (newtime.tv_usec - StartTime.tv_usec) / 1000.0;
		Frame = floor((ElapsedTime/1000.f)*Animations[AnimationID].GetFPS());
		Frame %= (Animations[AnimationID].ImageCount);
		glBindTexture(GL_TEXTURE_2D, Animations[AnimationID].Images[Frame]);
	}
	return Frame;
}

NTexture::Texture::~Texture()
{
	//free(Animations);
}

int NTexture::CleanUp()
{
	for (unsigned int i=0;i<TextureNames.size();i++)
	{
		free(TextureNames[i]);
	}
	TextureNames.clear();
	TextureIDs.clear();
	for (unsigned int i=0;i<Textures.size();i++)
	{
		if (!Textures[i].Static)
		{
			free(Textures[i].Animations);
		}
	}
	Textures.clear();
	return 0;
}