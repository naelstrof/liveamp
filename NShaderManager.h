#ifndef NAELSTROF_SHADER_MANAGER
#define NAELSTROF_SHADER_MANAGER

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
GLuint LoadShaders(const char* VertexFilePath,const char* FragmentFilePath);

#endif
