#pragma once
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

// Camera helpers
void updateCameraVectors();

// Custom perspective matrix (avoids GLU dependency)
glm::mat4 customPerspective(float fovY, float aspect, float zNear, float zFar);

// GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int w, int h);

// Integer-to-string helper
std::string itos(int v);

// Upload all per-frame lighting uniforms
void setupLighting(unsigned int sh);

// Print keyboard instructions to stdout
void printInstructions();