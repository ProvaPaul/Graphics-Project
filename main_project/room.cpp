// room.cpp  —  Bedroom rendered with the project's modern shader pipeline.
// No GLUT, no glMatrixMode, no glLightfv. Uses the same VAO/shader approach
// as the rest of the campus scene.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globals.h"   // SCR_WIDTH, SCR_HEIGHT, dayMode, inRoomView, camera
#include "shaders.h"   // createShaderProgram (already called in main)
#include "geometry.h"  // setupCube() / drawCube VAO helpers
#include "room.h"

#include <cmath>

// ── module globals ────────────────────────────────────────────────────────────
float brFanAngle = 0.0f;
bool  brLightOn = true;

extern int currentRoom;          // defined in main.cpp (PATCH 2)

// ── forward-declare the reading-room entry point (defined in reading_room.cpp)
void drawReadingRoom();

// ─────────────────────────────────────────────────────────────────────────────
//  INTERNAL HELPER  — draw one axis-aligned box via the project's cube VAO.
//
//  We replicate what scene.cpp / geometry.cpp already does:
//    • set "model" uniform
//    • set "objectColor" uniform  (or use texture flag = 0)
//    • call glDrawElements / glDrawArrays for the cube
//
//  Adjust the uniform names below to match YOUR actual shader variable names.
// ─────────────────────────────────────────────────────────────────────────────

// These must match the uniform names in your vertex/fragment shader.
// Common names used in student projects — change if yours differ.
static unsigned int g_shader = 0;   // cached shader handle, set in initRoom()

static void drawCube()
{
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void setColor(float r, float g, float b)
{
    glUniform3f(glGetUniformLocation(g_shader, "objectColor"), r, g, b);
    // Tell the shader to use a flat color, not a texture.
    glUniform1i(glGetUniformLocation(g_shader, "useTexture"), 0);
}

// Draw a box centred at (cx,cy,cz) with half-extents (sx,sy,sz)/2.
// Relies on the cube VAO already set up by setupCube() in main().
static void B(float cx, float cy, float cz,
    float sx, float sy, float sz)
{
    glm::mat4 model(1.f);
    model = glm::translate(model, glm::vec3(cx, cy, cz));
    model = glm::scale(model, glm::vec3(sx, sy, sz));
    glUniformMatrix4fv(glGetUniformLocation(g_shader, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    drawCube();   // your geometry.h function that binds the cube VAO & draws it
}

// ─────────────────────────────────────────────────────────────────────────────
//  LIGHTING  — uploads point-light uniforms for the bedroom.
//  Adjust uniform names to match your fragment shader.
// ─────────────────────────────────────────────────────────────────────────────
static void bedroomLighting()
{
    unsigned int s = g_shader;

    // Ambient
    glm::vec3 ambient = brLightOn
        ? glm::vec3(0.28f, 0.22f, 0.16f)
        : glm::vec3(0.06f, 0.05f, 0.04f);
    glUniform3fv(glGetUniformLocation(s, "ambientColor"), 1,
        glm::value_ptr(ambient));

    // Point light 0 — ceiling bulb
    if (brLightOn) {
        glUniform3f(glGetUniformLocation(s, "lightPos"), 0.f, 5.5f, 0.f);
        glUniform3f(glGetUniformLocation(s, "lightColor"), 1.0f, 0.90f, 0.65f);
        glUniform1f(glGetUniformLocation(s, "lightIntensity"), 1.0f);
    }
    else {
        glUniform3f(glGetUniformLocation(s, "lightPos"), 0.f, 5.5f, 0.f);
        glUniform3f(glGetUniformLocation(s, "lightColor"), 0.f, 0.f, 0.f);
        glUniform1f(glGetUniformLocation(s, "lightIntensity"), 0.0f);
    }

    // Point light 1 — nightstand lamp (always on)
    glUniform3f(glGetUniformLocation(s, "lightPos2"), -1.0f, 2.2f, -3.8f);
    glUniform3f(glGetUniformLocation(s, "lightColor2"), 1.0f, 0.72f, 0.35f);
    glUniform1f(glGetUniformLocation(s, "lightIntensity2"), 0.6f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ROOM GEOMETRY — same coordinates as the original; only the draw calls differ
// ─────────────────────────────────────────────────────────────────────────────

static void roomShell()
{
    // Floor — alternating oak-plank strips
    for (int i = 0; i < 10; i++) {
        float s = (i % 2 == 0) ? 0.50f : 0.44f;
        setColor(s * 1.5f, s * 0.88f, s * 0.44f);
        B(-4.5f + i * 1.0f + 0.5f, 0.0f, 0, 0.98f, 0.06f, 10.f);
    }

    // Ceiling
    setColor(0.93f, 0.92f, 0.90f);
    B(0, 6.0f, 0, 10.f, 0.08f, 10.f);

    // Back wall
    setColor(0.70f, 0.82f, 0.94f);
    B(0, 3.0f, -5.0f, 10.f, 6.f, 0.12f);

    // Left wall
    setColor(0.94f, 0.91f, 0.84f);
    B(-5.f, 3.f, 0, 0.12f, 6.f, 10.f);

    // Right wall
    setColor(0.94f, 0.91f, 0.84f);
    B(5.f, 3.f, 0, 0.12f, 6.f, 10.f);

    // Front wall (behind camera — gives depth)
    setColor(0.94f, 0.91f, 0.84f);
    B(0, 3.f, 5.0f, 10.f, 6.f, 0.12f);

    // Skirting boards
    setColor(0.30f, 0.18f, 0.09f);
    B(0, 0.18f, -4.95f, 10.f, 0.36f, 0.08f);
    B(-4.95f, 0.18f, 0, 0.08f, 0.36f, 10.f);
    B(4.95f, 0.18f, 0, 0.08f, 0.36f, 10.f);

    // Crown moulding
    setColor(0.90f, 0.88f, 0.85f);
    B(0, 5.84f, -4.95f, 10.f, 0.22f, 0.10f);
    B(-4.95f, 5.84f, 0, 0.10f, 0.22f, 10.f);
    B(4.95f, 5.84f, 0, 0.10f, 0.22f, 10.f);
}

static void drawWindow()
{
    float wx = -1.8f, wy = 3.6f, wz = -4.92f;

    setColor(0.56f, 0.78f, 0.98f);   // sky
    B(wx, wy, wz - 0.02f, 2.2f, 1.9f, 0.04f);

    setColor(0.95f, 0.94f, 0.92f);   // white frame
    B(wx, wy, wz, 2.4f, 2.1f, 0.12f);

    setColor(0.74f, 0.87f, 0.97f);   // glass
    B(wx, wy, wz + 0.03f, 2.0f, 1.7f, 0.04f);

    setColor(0.88f, 0.87f, 0.85f);   // mullion
    B(wx, wy, wz + 0.05f, 0.06f, 1.7f, 0.05f);
    B(wx, wy, wz + 0.05f, 2.0f, 0.06f, 0.05f);

    setColor(0.46f, 0.34f, 0.20f);   // curtain rod
    B(wx, wy + 1.20f, wz + 0.18f, 3.0f, 0.08f, 0.08f);

    setColor(0.76f, 0.52f, 0.56f);   // curtains (dusty rose)
    B(wx - 0.94f, wy, wz + 0.18f, 0.56f, 2.2f, 0.09f);
    B(wx + 0.94f, wy, wz + 0.18f, 0.56f, 2.2f, 0.09f);
}

static void drawDoor()
{
    setColor(0.48f, 0.34f, 0.18f);   // frame
    B(4.93f, 2.6f, 3.2f, 0.14f, 5.2f, 1.12f);

    setColor(0.56f, 0.40f, 0.22f);   // panel
    B(4.91f, 2.5f, 3.2f, 0.09f, 5.0f, 0.96f);

    setColor(0.50f, 0.36f, 0.18f);   // recessed panels
    B(4.91f, 3.4f, 3.2f, 0.06f, 1.5f, 0.72f);
    B(4.91f, 1.5f, 3.2f, 0.06f, 1.4f, 0.72f);

    setColor(0.80f, 0.64f, 0.34f);   // handle
    B(4.90f, 2.55f, 2.82f, 0.06f, 0.08f, 0.32f);
    B(4.90f, 2.55f, 2.72f, 0.06f, 0.22f, 0.06f);
}

static void drawBed()
{
    float bx = -2.4f, bz = -1.5f;

    setColor(0.26f, 0.16f, 0.07f);   // legs
    float fp[4][2] = { {bx - 0.9f,bz - 2.0f},{bx + 0.9f,bz - 2.0f},
                      {bx - 0.9f,bz + 2.0f},{bx + 0.9f,bz + 2.0f} };
    for (auto& f : fp) B(f[0], 0.14f, f[1], 0.13f, 0.28f, 0.13f);

    setColor(0.30f, 0.18f, 0.08f);
    B(bx - 0.90f, 0.35f, bz, 0.13f, 0.42f, 4.1f);
    B(bx + 0.90f, 0.35f, bz, 0.13f, 0.42f, 4.1f);

    setColor(0.38f, 0.26f, 0.14f);
    for (int s = 0; s < 7; s++)
        B(bx, 0.22f, bz - 1.8f + s * 0.62f, 1.66f, 0.06f, 0.18f);

    // ── Headboard now at the BACK-WALL side (bz-2.10, toward -Z) ──
    setColor(0.14f, 0.18f, 0.32f);
    B(bx, 1.12f, bz - 2.10f, 1.88f, 1.82f, 0.20f);
    setColor(0.10f, 0.14f, 0.26f);
    for (int r = 0; r < 3; r++) for (int c = 0; c < 4; c++)
        B(bx - 0.62f + c * 0.42f, 0.52f + r * 0.44f, bz - 2.18f, 0.09f, 0.09f, 0.06f);
    setColor(0.24f, 0.16f, 0.08f);
    B(bx, 1.96f, bz - 2.10f, 1.88f, 0.24f, 0.24f);

    // ── Footboard now at the CAMERA side (bz+2.05) ──
    setColor(0.24f, 0.15f, 0.07f);
    B(bx, 0.56f, bz + 2.05f, 1.88f, 0.80f, 0.18f);

    setColor(0.94f, 0.92f, 0.90f);   // mattress
    B(bx, 0.60f, bz, 1.72f, 0.42f, 4.0f);
    setColor(0.72f, 0.82f, 0.92f);   // fitted sheet
    B(bx, 0.84f, bz, 1.70f, 0.04f, 3.98f);
    setColor(0.96f, 0.93f, 0.88f);   // duvet (toward footboard)
    B(bx, 0.96f, bz + 0.5f, 1.68f, 0.26f, 3.00f);
    setColor(0.80f, 0.58f, 0.60f);
    B(bx, 0.96f, bz - 0.74f, 1.68f, 0.28f, 0.26f);  // decorative stripe near head

    // ── Pillows near headboard (back-wall side) ──
    setColor(0.96f, 0.95f, 0.94f);
    B(bx - 0.44f, 0.96f, bz - 1.66f, 0.72f, 0.22f, 0.92f);
    B(bx + 0.44f, 0.96f, bz - 1.66f, 0.72f, 0.22f, 0.92f);

    // Nightstand beside headboard (back-wall side)
    float nx = bx + 1.38f, nz = bz - 1.60f;
    setColor(0.30f, 0.20f, 0.10f);
    B(nx, 0.56f, nz, 0.60f, 1.12f, 0.62f);
    setColor(0.76f, 0.62f, 0.36f);
    B(nx, 0.46f, nz + 0.35f, 0.16f, 0.05f, 0.04f);

    // Table lamp
    setColor(0.44f, 0.38f, 0.28f);
    B(nx, 1.20f, nz, 0.13f, 0.20f, 0.13f);
    setColor(0.96f, 0.88f, 0.66f);
    B(nx, 1.62f, nz, 0.38f, 0.28f, 0.38f);
    setColor(1.f, 0.97f, 0.76f);
    B(nx, 1.54f, nz, 0.12f, 0.10f, 0.12f);
}

static void drawWardrobe()
{
    float wx = -4.80f, wz = 1.8f;
    setColor(0.26f, 0.16f, 0.08f);
    B(wx, 3.0f, wz, 0.52f, 6.0f, 2.92f);
    setColor(0.34f, 0.22f, 0.12f);
    B(wx + 0.27f, 3.0f, wz - 0.72f, 0.05f, 5.62f, 1.38f);
    B(wx + 0.27f, 3.0f, wz + 0.72f, 0.05f, 5.62f, 1.38f);
    setColor(0.76f, 0.62f, 0.36f);
    B(wx + 0.29f, 3.0f, wz - 0.20f, 0.04f, 0.26f, 0.05f);
    B(wx + 0.29f, 3.0f, wz + 0.20f, 0.04f, 0.26f, 0.05f);
    setColor(0.22f, 0.13f, 0.06f);
    B(wx + 0.02f, 6.12f, wz, 0.56f, 0.22f, 2.96f);
}

static void drawStudyTable()
{
    float tx = 3.0f, tz = -2.6f;

    setColor(0.72f, 0.56f, 0.34f);
    B(tx, 1.52f, tz, 1.92f, 0.08f, 0.98f);

    setColor(0.66f, 0.50f, 0.30f);   // drawer unit
    B(tx + 0.66f, 0.84f, tz, 0.60f, 1.48f, 0.86f);
    for (int d = 0; d < 3; d++) {
        setColor(0.62f, 0.48f, 0.28f);
        B(tx + 0.66f, 0.32f + d * 0.47f, tz + 0.44f, 0.54f, 0.38f, 0.05f);
        setColor(0.76f, 0.62f, 0.40f);
        B(tx + 0.66f, 0.32f + d * 0.47f, tz + 0.47f, 0.16f, 0.05f, 0.04f);
    }

    setColor(0.56f, 0.42f, 0.24f);   // legs
    B(tx - 0.86f, 0.76f, tz - 0.44f, 0.08f, 1.50f, 0.08f);
    B(tx - 0.86f, 0.76f, tz + 0.44f, 0.08f, 1.50f, 0.08f);

    setColor(0.38f, 0.26f, 0.14f);   // mirror frame
    B(tx, 2.92f, tz - 0.47f, 1.24f, 1.54f, 0.10f);
    setColor(0.76f, 0.88f, 0.94f);
    B(tx, 2.92f, tz - 0.44f, 1.08f, 1.34f, 0.05f);

    // Books on desk
    float bc[3][3] = { {0.60f,0.22f,0.18f},{0.24f,0.40f,0.58f},{0.28f,0.52f,0.22f} };
    for (int b = 0; b < 3; b++) {
        setColor(bc[b][0], bc[b][1], bc[b][2]);
        B(tx - 0.44f, 1.58f + b * 0.08f, tz - 0.12f, 0.30f, 0.08f, 0.42f);
    }

    // Desk lamp
    setColor(0.90f, 0.78f, 0.30f);
    B(tx + 0.60f, 1.98f, tz + 0.20f, 0.32f, 0.18f, 0.24f);
    setColor(1.f, 0.97f, 0.78f);
    B(tx + 0.60f, 1.92f, tz + 0.20f, 0.10f, 0.08f, 0.10f);

    // Chair — seat toward table (-Z), backrest behind seat (+Z side)
    float cx = 3.0f, ccz = -1.4f;
    setColor(0.20f, 0.42f, 0.48f);
    B(cx, 1.10f, ccz, 0.68f, 0.12f, 0.66f);           // seat
    B(cx, 1.56f, ccz + 0.30f, 0.68f, 0.76f, 0.12f);     // backrest BEHIND seat (away from table)
    setColor(0.24f, 0.19f, 0.14f);
    B(cx - 0.38f, 1.36f, ccz + 0.08f, 0.07f, 0.44f, 0.52f);  // armrests
    B(cx + 0.38f, 1.36f, ccz + 0.08f, 0.07f, 0.44f, 0.52f);
    setColor(0.52f, 0.52f, 0.55f);
    B(cx, 0.57f, ccz, 0.11f, 1.08f, 0.11f);            // central post
}

static void drawCeilingFan()
{
    float fY2 = 4.90f;
    float t = (float)glfwGetTime();
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, fY2 + 0.96f, 0.f }), { 0.28f, 0.18f, 0.28f });
    setColor(0.50f, 0.50f, 0.52f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, fY2 + 0.96f, 0.f }), { 0.28f, 0.18f, 0.28f });
    glUniformMatrix4fv(glGetUniformLocation(g_shader, "model"), 1, GL_FALSE, glm::value_ptr(m));
    drawCube();
    for (int b = 0; b < 4; b++) {
        float ang = t * 100.f + b * 90.f;
        glm::mat4 blade = glm::translate(glm::mat4(1.f), { 0.f, fY2 + 0.95f, 0.f });
        blade = glm::rotate(blade, glm::radians(ang), { 0.f, 1.f, 0.f });
        blade = glm::translate(blade, { 1.0f, 0.f, 0.f });
        blade = glm::scale(blade, { 1.50f, 0.05f, 0.38f });
        setColor(0.48f, 0.38f, 0.24f);
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "model"), 1, GL_FALSE, glm::value_ptr(blade));
        drawCube();
    }
    setColor(0.50f, 0.50f, 0.52f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f, fY2 + 1.06f, 0.f }), { 0.05f, 0.18f, 0.05f });
    glUniformMatrix4fv(glGetUniformLocation(g_shader, "model"), 1, GL_FALSE, glm::value_ptr(m));
    drawCube();
}

static void drawWallArt()
{
    setColor(0.32f, 0.22f, 0.12f);
    B(1.8f, 3.8f, -4.92f, 1.14f, 0.90f, 0.09f);
    setColor(0.80f, 0.62f, 0.44f);
    B(1.8f, 3.8f, -4.89f, 0.98f, 0.72f, 0.05f);
}

static void drawRug()
{
    setColor(0.46f, 0.10f, 0.14f);
    B(0.5f, 0.04f, 0.5f, 4.2f, 0.04f, 3.0f);
    setColor(0.68f, 0.54f, 0.28f);
    B(0.5f, 0.05f, 0.5f, 3.80f, 0.04f, 2.60f);
    setColor(0.44f, 0.09f, 0.13f);
    B(0.5f, 0.06f, 0.5f, 3.20f, 0.04f, 2.00f);
}

static void drawPlant()
{
    setColor(0.70f, 0.36f, 0.20f);
    B(-4.2f, 0.44f, -4.2f, 0.34f, 0.58f, 0.34f);
    setColor(0.18f, 0.50f, 0.22f);
    float lx[5] = { -4.2f,-4.0f,-4.4f,-4.15f,-4.25f };
    float lz[5] = { -4.2f,-4.0f,-4.1f,-4.38f,-4.05f };
    float lh[5] = { 1.0f, 0.8f, 0.9f, 0.7f,  1.1f };
    for (int i = 0; i < 5; i++) {
        B(lx[i], 0.76f + lh[i] * 0.5f, lz[i], 0.08f, lh[i], 0.08f);
        setColor(0.22f, 0.58f, 0.26f);
        B(lx[i] + 0.06f, 0.76f + lh[i], lz[i] + 0.06f, 0.16f, 0.13f, 0.16f);
        setColor(0.18f, 0.50f, 0.22f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  PUBLIC API
// ─────────────────────────────────────────────────────────────────────────────

// Called once at startup from main() to cache the shader handle.
void initRoom()
{
    // The shader is already created and bound in main(); we just need its ID.
    // Retrieve the currently-bound shader program.
    GLint cur = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
    g_shader = (unsigned int)cur;
}

// Called every frame when inRoomView && currentRoom == 0.
// The projection + view matrices are already uploaded by main().
void drawBedroom()
{
    // Make sure we have the shader handle (safe re-fetch every frame)
    GLint cur = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
    g_shader = (unsigned int)cur;

    bedroomLighting();

    roomShell();
    drawWindow();
    drawDoor();
    drawRug();
    drawBed();
    drawWardrobe();
    drawStudyTable();
    drawCeilingFan();
    drawWallArt();
    drawPlant();
}

void updateBedroomFan()
{
    brFanAngle += 2.8f;
    if (brFanAngle >= 360.f) brFanAngle -= 360.f;
}

void toggleBedroomLight()
{
    brLightOn = !brLightOn;
}

// Dispatcher used by renderScene() — kept for compatibility.
   extern void drawDiningHall();   // defined in dormitory_dining.cpp
   extern void drawWashroom(unsigned int sh);  // defined in washroom.cpp

   void renderRoom(unsigned int shader)
   {
       g_shader = shader;
       if (currentRoom == 0)      drawBedroom();
       else if (currentRoom == 1) drawReadingRoom();
       else if (currentRoom == 2) drawDiningHall();
       else if (currentRoom == 8) drawWashroom(shader);
   }