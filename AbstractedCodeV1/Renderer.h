#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <alloca.h>
#include <cmath>
#include <csignal>

#define ASSERT(x) if (!(x)) raise(SIGTRAP); 
#define GLCall(x) GLClearError();\
  x;\
  ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

