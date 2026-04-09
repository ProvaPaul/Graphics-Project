#include "corridor.h"
#include "draw_helpers.h"
#include "globals.h"
#include "utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

static unsigned int corr_sh = 0;
static float corrFanAngle = 0.0f;
static bool corrLightOn = true;
static int selectedDoor = -1;

static const float CORR_LEN = 40.0f;
static const float CORR_W = 6.0f;
static const float CORR_H = 4.0f;
static const float WALL_T = 0.15f;

// ── Door type enum (all 8 rooms) ──────────────────────────────────────────────
enum DoorType {
    DOOR_BEDROOM1, DOOR_BEDROOM2, DOOR_BEDROOM3, DOOR_BEDROOM4,
    DOOR_DINING, DOOR_READING, DOOR_TV, DOOR_WASHROOM
};

struct Door {
    DoorType    type;
    float       posZ;
    bool        isLeft;
    const char* name;
};

// ── 8 doors: 4 left (bedrooms) + 4 right (dining, reading, TV, washroom) ─────
static Door doors[] = {
    {DOOR_BEDROOM1, -15.f, true,  "DOOR 1 - BEDROOM 1"},
    {DOOR_BEDROOM2,  -5.f, true,  "DOOR 2 - BEDROOM 2"},
    {DOOR_BEDROOM3,   5.f, true,  "DOOR 3 - BEDROOM 3"},
    {DOOR_BEDROOM4,  15.f, true,  "DOOR 4 - BEDROOM 4"},
    {DOOR_DINING,   -12.f, false, "DOOR 5 - DINING HALL"},
    {DOOR_READING,    4.f, false, "DOOR 6 - READING ROOM"},
    {DOOR_TV,        10.f, false, "DOOR 7 - TV LOUNGE"},
    {DOOR_WASHROOM,  16.f, false, "DOOR 8 - WASHROOM"}
};

static const int NUM_DOORS = (int)(sizeof(doors) / sizeof(doors[0])); // 8

// ─────────────────────────────────────────────────────────────────────────────
static void corrDrawCube(glm::mat4 m, glm::vec3 col,
    unsigned int tex = 0, float tile = 1.f)
{
    glUniformMatrix4fv(glGetUniformLocation(corr_sh, "model"),
        1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(corr_sh, "objectColor"),
        1, glm::value_ptr(col));
    if (tex != 0) {
        glUniform1i(glGetUniformLocation(corr_sh, "hasTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(corr_sh, "texSampler"), 0);
        glUniform1f(glGetUniformLocation(corr_sh, "uvTile"), tile);
    }
    else {
        glUniform1i(glGetUniformLocation(corr_sh, "hasTexture"), 0);
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void corrBox(float cx, float cy, float cz,
    float sx, float sy, float sz,
    glm::vec3 col, unsigned int tex = 0, float tile = 1.f)
{
    glm::mat4 m = glm::translate(glm::mat4(1.f), { cx, cy, cz });
    m = glm::scale(m, { sx, sy, sz });
    corrDrawCube(m, col, tex, tile);
}

// ─────────────────────────────────────────────────────────────────────────────
static void corrLighting()
{
    glm::vec3 amb = corrLightOn
        ? glm::vec3(0.40f, 0.38f, 0.32f)
        : glm::vec3(0.08f, 0.07f, 0.06f);
    glUniform3fv(glGetUniformLocation(corr_sh, "ambientColor"),
        1, glm::value_ptr(amb));

    if (corrLightOn) {
        glUniform3f(glGetUniformLocation(corr_sh, "lightPos"),
            0.f, CORR_H - 0.3f, 0.f);
        glUniform3f(glGetUniformLocation(corr_sh, "lightColor"),
            1.0f, 0.95f, 0.80f);
        glUniform1f(glGetUniformLocation(corr_sh, "lightIntensity"), 1.5f);

        glUniform3f(glGetUniformLocation(corr_sh, "lightPos2"),
            0.f, CORR_H - 0.3f, -CORR_LEN * 0.5f + 5.f);
        glUniform3f(glGetUniformLocation(corr_sh, "lightColor2"),
            1.0f, 0.92f, 0.75f);
        glUniform1f(glGetUniformLocation(corr_sh, "lightIntensity2"), 0.8f);
    }
    else {
        glUniform3f(glGetUniformLocation(corr_sh, "lightPos"),
            0.f, CORR_H - 0.3f, 0.f);
        glUniform3f(glGetUniformLocation(corr_sh, "lightColor"),
            0.f, 0.f, 0.f);
        glUniform1f(glGetUniformLocation(corr_sh, "lightIntensity"), 0.f);

        glUniform3f(glGetUniformLocation(corr_sh, "lightPos2"),
            0.f, CORR_H - 0.3f, -CORR_LEN * 0.5f + 5.f);
        glUniform3f(glGetUniformLocation(corr_sh, "lightColor2"),
            0.f, 0.f, 0.f);
        glUniform1f(glGetUniformLocation(corr_sh, "lightIntensity2"), 0.f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawCorridorShell()
{
    glm::vec3 floorCol(0.45f, 0.40f, 0.35f);
    glm::vec3 wallCol(0.80f, 0.76f, 0.70f);
    glm::vec3 ceilCol(0.88f, 0.86f, 0.82f);
    glm::vec3 trimCol(0.60f, 0.55f, 0.50f);

    float halfLen = CORR_LEN * 0.5f;

    // Floor & ceiling
    corrBox(0.f, 0.f, 0.f, CORR_W, WALL_T, CORR_LEN, floorCol, texTile, 2.f);
    corrBox(0.f, CORR_H, 0.f, CORR_W, WALL_T, CORR_LEN, ceilCol, texConcrete, 3.f);

    // Side walls
    corrBox(-CORR_W * 0.5f - WALL_T * 0.5f, CORR_H * 0.5f, 0.f,
        WALL_T, CORR_H, CORR_LEN, wallCol, texConcrete, 2.f);
    corrBox(CORR_W * 0.5f + WALL_T * 0.5f, CORR_H * 0.5f, 0.f,
        WALL_T, CORR_H, CORR_LEN, wallCol, texConcrete, 2.f);

    // End walls
    corrBox(0.f, CORR_H * 0.5f, -halfLen,
        CORR_W + WALL_T * 2.f, CORR_H, WALL_T, wallCol, texConcrete, 2.f);
    corrBox(0.f, CORR_H * 0.5f, halfLen,
        CORR_W + WALL_T * 2.f, CORR_H, WALL_T, wallCol, texConcrete, 2.f);

    // Ceiling trim strips
    for (float z = -halfLen + 5.f; z < halfLen - 5.f; z += 10.f) {
        corrBox(0.f, CORR_H - 0.2f, z,
            CORR_W - 0.5f, WALL_T * 0.3f, WALL_T * 0.3f, trimCol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawCeilingLights()
{
    float spacing = CORR_LEN / 6.f;
    const glm::vec3 zero(0.f);

    for (int i = 0; i < 6; i++) {
        float z = -CORR_LEN * 0.5f + spacing * 0.5f + i * spacing;

        corrBox(0.f, CORR_H - 0.05f, z,
            1.5f, 0.06f, 1.0f, glm::vec3(0.92f, 0.90f, 0.85f));

        if (corrLightOn) {
            glm::vec3 emit(0.8f, 0.75f, 0.60f);
            glUniform3fv(glGetUniformLocation(corr_sh, "emissive"),
                1, glm::value_ptr(emit));
        }

        corrBox(0.f, CORR_H - 0.1f, z,
            0.8f, 0.04f, 0.6f, glm::vec3(1.0f, 0.95f, 0.80f));

        // Reset emissive after each bulb
        glUniform3fv(glGetUniformLocation(corr_sh, "emissive"),
            1, glm::value_ptr(zero));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawDoorFrame(float x, float z, bool isHighlight)
{
    glm::vec3 frameCol = isHighlight
        ? glm::vec3(0.90f, 0.75f, 0.30f) : glm::vec3(0.50f, 0.32f, 0.15f);
    glm::vec3 doorCol = isHighlight
        ? glm::vec3(0.70f, 0.55f, 0.25f) : glm::vec3(0.55f, 0.38f, 0.20f);
    glm::vec3 handleCol = isHighlight
        ? glm::vec3(1.0f, 0.90f, 0.40f) : glm::vec3(0.80f, 0.70f, 0.50f);

    float doorW = 1.4f;
    float doorH = 3.2f;
    float side = x < 0.f ? -1.f : 1.f;

    float wallSurface = side * (CORR_W * 0.5f);

    // Frame
    corrBox(wallSurface, doorH * 0.5f + 0.1f, z,
        WALL_T, doorH + 0.3f, doorW + 0.4f, frameCol, texWood, 1.f);

    // Door panel
    corrBox(wallSurface + side * WALL_T * 0.6f, doorH * 0.5f + 0.1f, z,
        WALL_T * 0.4f, doorH, doorW, doorCol, texWood, 1.f);

    // Handle
    corrBox(wallSurface + side * WALL_T * 0.6f, 1.8f, z + doorW * 0.35f,
        WALL_T * 0.5f, 0.06f, 0.18f, handleCol);

    // Highlight strip above door
    if (isHighlight) {
        corrBox(wallSurface, doorH + 0.35f, z,
            WALL_T * 1.2f, 0.15f, doorW + 0.6f,
            glm::vec3(1.0f, 0.95f, 0.50f));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawAllDoors()
{
    for (int i = 0; i < NUM_DOORS; i++) {
        bool isHighlight = (selectedDoor == i);
        drawDoorFrame(doors[i].isLeft ? -1.f : 1.f, doors[i].posZ, isHighlight);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawDoorSigns()
{
    for (int i = 0; i < NUM_DOORS; i++) {
        float signX = doors[i].isLeft
            ? -CORR_W * 0.5f - WALL_T * 1.2f
            : CORR_W * 0.5f + WALL_T * 1.2f;
        float z = doors[i].posZ;

        glm::vec3 signCol = (selectedDoor == i)
            ? glm::vec3(0.90f, 0.75f, 0.20f) : glm::vec3(0.20f, 0.18f, 0.15f);
        glm::vec3 textCol = (selectedDoor == i)
            ? glm::vec3(0.10f, 0.08f, 0.05f) : glm::vec3(0.90f, 0.88f, 0.85f);

        corrBox(signX, CORR_H - 0.6f, z, 0.08f, 0.6f, 1.8f, signCol);
        corrBox(signX, CORR_H - 0.6f, z, 0.04f, 0.5f, 1.6f, textCol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void drawCorridorFan()
{
    float fY2 = CORR_H - 0.4f;
    float t = (float)glfwGetTime();

    // Motor / hub
    corrBox(0.f, fY2 + 0.96f, 0.f,
        0.28f, 0.18f, 0.28f, glm::vec3(0.50f, 0.50f, 0.52f));

    // 4 rotating blades
    for (int b = 0; b < 4; b++) {
        float ang = t * 100.f + b * 90.f;
        glm::mat4 blade = glm::translate(glm::mat4(1.f),
            { 0.f, fY2 + 0.95f, 0.f });
        blade = glm::rotate(blade, glm::radians(ang), { 0.f, 1.f, 0.f });
        blade = glm::translate(blade, { 1.0f, 0.f, 0.f });
        blade = glm::scale(blade, { 1.50f, 0.05f, 0.38f });
        corrDrawCube(blade, glm::vec3(0.48f, 0.38f, 0.24f));
    }

    // Rod connecting fan to ceiling
    corrBox(0.f, fY2 + 1.06f, 0.f,
        0.05f, 0.18f, 0.05f, glm::vec3(0.50f, 0.50f, 0.52f));
}

// ─────────────────────────────────────────────────────────────────────────────
void initCorridor()
{
    camera.position = glm::vec3(0.f, 1.8f, CORR_LEN * 0.5f - 2.f);
    camera.front = glm::vec3(0.f, 0.f, -1.f);
    camera.up = glm::vec3(0.f, 1.f, 0.f);
    camera.yaw = -90.f;
    camera.pitch = 0.f;
    camera.roll = 0.f;
    camera.speed = 4.f;
    updateCameraVectors();
    selectedDoor = -1;

    printf("\n");
    printf("====================================================\n");
    printf("           CORRIDOR - DORMITORY ENTRANCE\n");
    printf("====================================================\n");
    printf("  You are now in the main corridor.\n");
    printf("  LEFT SIDE:  4 Bedroom doors\n");
    printf("  RIGHT SIDE: Dining, Reading, TV, Washroom\n");
    printf("----------------------------------------------------\n");
    printf("  W/S/A/D - Walk around corridor\n");
    printf("  E       - Enter selected door\n");
    printf("  P       - Return from room to corridor\n");
    printf("  B       - Return to campus exterior\n");
    printf("  L       - Toggle corridor light\n");
    printf("====================================================\n\n");
    fflush(stdout);
}

// ─────────────────────────────────────────────────────────────────────────────
void updateCorridor()
{
    float halfLen = CORR_LEN * 0.5f - 1.5f;
    float halfW = CORR_W * 0.5f - 0.8f;

    camera.position.z = glm::clamp(camera.position.z, -halfLen, halfLen);
    camera.position.x = glm::clamp(camera.position.x, -halfW, halfW);
    camera.position.y = 1.8f;

    selectedDoor = -1;
    float nearestDist = 5.0f;

    for (int i = 0; i < NUM_DOORS; i++) {
        float dz = camera.position.z - doors[i].posZ;
        float dist = fabsf(dz);

        float doorX = doors[i].isLeft ? -halfW : halfW;
        float dx = camera.position.x - doorX;

        if (dist < nearestDist && fabsf(dx) < 3.0f) {
            nearestDist = dist;
            selectedDoor = i;
        }
    }

    if (selectedDoor >= 0) {
        printf("\r[DOOR] %s - Press E to enter    ", doors[selectedDoor].name);
    }
    else {
        printf("\r                                              ");
    }
    fflush(stdout);
}

// ─────────────────────────────────────────────────────────────────────────────
int getSelectedDoorRoom()
{
    if (selectedDoor < 0) return -1;
    switch (doors[selectedDoor].type) {
    case DOOR_BEDROOM1: return 0;   // Bedroom 1
    case DOOR_BEDROOM2: return 1;   // Bedroom 2
    case DOOR_BEDROOM3: return 2;   // Bedroom 3
    case DOOR_BEDROOM4: return 3;   // Bedroom 4
    case DOOR_DINING:   return 4;   // Dining Hall
    case DOOR_READING:  return 5;   // Reading Room
    case DOOR_TV:       return 6;   // TV Lounge
    case DOOR_WASHROOM: return 8;   // Washroom
    default:            return -1;
    }
}

const char* getSelectedDoorName()
{
    if (selectedDoor < 0) return "";
    return doors[selectedDoor].name;
}

// ─────────────────────────────────────────────────────────────────────────────
void drawCorridor()
{
    GLint cur = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
    corr_sh = (unsigned int)cur;

    corrLighting();
    drawCorridorShell();
    drawCeilingLights();
    drawAllDoors();
    drawDoorSigns();
    drawCorridorFan();
}

void updateCorridorFan()
{
    corrFanAngle += 1.8f;
    if (corrFanAngle >= 360.f) corrFanAngle -= 360.f;
}

void toggleCorridorLight()
{
    corrLightOn = !corrLightOn;
}