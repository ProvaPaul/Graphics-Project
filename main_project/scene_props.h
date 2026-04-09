#pragma once
#pragma once
#include <glm/glm.hpp>
// Add this line alongside the other extern declarations:
void renderFractalTrees(unsigned int sh);
void renderPalmTree(unsigned int sh, glm::vec3 base);
void renderConeTree(unsigned int sh, float x, float z);
void renderBench(unsigned int sh, glm::vec3 pos, float rotY = 0.f);
void renderFountain(unsigned int sh, glm::vec3 pos);
void renderLampPost(unsigned int sh, float x, float z);
void renderCourtyard(unsigned int sh);
void renderEntranceSphere(unsigned int sh);
void renderGate(unsigned int sh);
void renderTelevision(unsigned int sh);
