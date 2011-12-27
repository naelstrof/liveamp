#ifndef NAELSTROF_SHADER_MANAGER
#define NAELSTROF_SHADER_MANAGER

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
GLuint LoadShaders(const char* VertexFilePath,const char* FragmentFilePath, bool* Result);

#endif
