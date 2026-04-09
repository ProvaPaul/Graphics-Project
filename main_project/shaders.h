#pragma once
#include <glad/glad.h>

extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;

unsigned int compileShader(unsigned int type, const char* src);
unsigned int createShaderProgram();