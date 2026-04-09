#pragma once
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

// Sets texture uniforms on the shader before a draw call.
void applyTexture(unsigned int shader, unsigned int texID, float uvTile);

// Basic primitive draw calls
void drawCube(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex = 0, float tile = 1.f);
void drawGlass(unsigned int sh, glm::mat4 m);
void drawCrystalGlass(unsigned int sh, glm::mat4 m);
void drawSignCube(unsigned int sh, glm::mat4 m);
void drawCylinder(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex = 0, float tile = 1.f);
void drawSphere(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex = 0, float tile = 1.f);
void drawCone(unsigned int sh, glm::mat4 m, glm::vec3 col, unsigned int tex = 0, float tile = 1.f);