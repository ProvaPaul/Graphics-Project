#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
// ── Room view toggle ──────────────────────────────────────────────────
extern bool inRoomView;
extern glm::vec3 savedCameraPos;   // saves position before entering room
extern float     savedYaw;
extern float     savedPitch;
// ─── Screen ──────────────────────────────────────────────────────────────────
extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern int activeViewport;

// ─── Timing ──────────────────────────────────────────────────────────────────
extern float deltaTime;
extern float lastFrame;

// ─── Camera ──────────────────────────────────────────────────────────────────
struct Camera {
    glm::vec3 position{ 0.f, 22.f, 105.f };
    glm::vec3 front{ 0.f,  0.f,  -1.f };
    glm::vec3 up{ 0.f,  1.f,   0.f };
    glm::vec3 right{ 1.f,  0.f,   0.f };
    glm::vec3 worldUp{ 0.f,  1.f,   0.f };
    float yaw = -90.f;
    float pitch = 0.f;
    float roll = 0.f;
    float speed = 5.f;
    bool birdsEyeMode = false;
    bool orbitMode = false;
};

// ─── Lighting State ───────────────────────────────────────────────────────────
struct LightingState {
    bool directionalLightOn = true;
    bool pointLightsOn = true;
    bool spotLightOn = true;
    bool ambientOn = true;
    bool diffuseOn = true;
    bool specularOn = true;
};

extern Camera       camera;
extern LightingState lighting;
extern bool  dayMode;
extern int   textureMode;

// ─── Texture IDs ─────────────────────────────────────────────────────────────
extern unsigned int texBrick;
extern unsigned int texGrass;
extern unsigned int texConcrete;
extern unsigned int texMarble;
extern unsigned int texWood;
extern unsigned int texTile;

// ─── VAO / VBO handles ───────────────────────────────────────────────────────
extern unsigned int cubeVAO, cubeVBO;
extern unsigned int cylVAO, cylVBO;  extern int cylCount;
extern unsigned int sphVAO, sphVBO;  extern int sphCount;
extern unsigned int conVAO, conVBO;  extern int conCount;

// ─── Spline railing ──────────────────────────────────────────────────────────
extern unsigned int splineVAO;
extern unsigned int splineVBO;
extern int          splineVertCount;

// ─── Ruled surface roof ──────────────────────────────────────────────────────
extern unsigned int ruledVAO;
extern unsigned int ruledVBO;
extern unsigned int ruledEBO;
extern int          ruledIndexCount;

// ─── Colour palette ──────────────────────────────────────────────────────────
extern const glm::vec3 COL_CONCRETE;
extern const glm::vec3 COL_ROOF;
extern const glm::vec3 COL_COLUMN;
extern const glm::vec3 COL_REDBRICK;
extern const glm::vec3 COL_GLASS;
extern const glm::vec3 COL_DOOR;
extern const glm::vec3 COL_PLAZA;
extern const glm::vec3 COL_GRASS;
extern const glm::vec3 COL_TRUNK;
extern const glm::vec3 COL_PALM;
extern const glm::vec3 COL_SIGN;
extern const glm::vec3 COL_BEAM;
extern const glm::vec3 COL_MARBLE;
extern const glm::vec3 COL_STONE;
extern const glm::vec3 COL_LETTER;
extern const glm::vec3 COL_WATER;
extern const glm::vec3 COL_RAILING;
extern const glm::vec3 COL_FLOOR;
extern const glm::vec3 COL_BALCONY;

// ─── PI constant ─────────────────────────────────────────────────────────────
extern const float PI;