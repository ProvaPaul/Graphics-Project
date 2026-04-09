#include "tvroom.h"
#include "draw_helpers.h"
#include "globals.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

static const float RY = -1.0f;
static const float CEIL_Y = 5.0f;
static const float ROOM_H = 6.0f;

static const glm::vec3 C_WALL(0.85f, 0.82f, 0.78f);
static const glm::vec3 C_WALL_ACCENT(0.75f, 0.70f, 0.65f);
static const glm::vec3 C_FLOOR(0.72f, 0.55f, 0.40f);
static const glm::vec3 C_CEIL(0.95f, 0.95f, 0.93f);
static const glm::vec3 C_TRIM(0.92f, 0.88f, 0.82f);
static const glm::vec3 C_WOOD_DK(0.28f, 0.15f, 0.08f);
static const glm::vec3 C_WOOD_MD(0.45f, 0.25f, 0.12f);
static const glm::vec3 C_WOOD_LT(0.60f, 0.40f, 0.22f);
static const glm::vec3 C_CUSHION(0.55f, 0.35f, 0.25f);
static const glm::vec3 C_CUSHION2(0.40f, 0.50f, 0.55f);
static const glm::vec3 C_TV_SCRN(0.01f, 0.01f, 0.03f);
static const glm::vec3 C_TV_BODY(0.12f, 0.12f, 0.14f);
static const glm::vec3 C_METAL(0.75f, 0.70f, 0.65f);
static const glm::vec3 C_GOLD(0.85f, 0.72f, 0.35f);
static const glm::vec3 C_LAMP(0.95f, 0.92f, 0.85f);
static const glm::vec3 C_GLASS(0.60f, 0.75f, 0.70f);
static const glm::vec3 C_PLANT(0.25f, 0.45f, 0.20f);
static const glm::vec3 C_POT(0.70f, 0.45f, 0.35f);
static const glm::vec3 C_DOOR(0.50f, 0.30f, 0.15f);
static const glm::vec3 C_WHITE(0.95f, 0.95f, 0.95f);
static const glm::vec3 C_SHADOW(0.15f, 0.12f, 0.10f);
static const glm::vec3 C_GLOW_WARM(1.0f, 0.85f, 0.60f);
static const glm::vec3 C_GLOW_COOL(0.70f, 0.80f, 1.0f);
static const glm::vec3 C_ART1(0.20f, 0.25f, 0.40f);
static const glm::vec3 C_ART2(0.60f, 0.30f, 0.35f);

static void box(unsigned int sh, float cx, float cy, float cz, float hw, float hh, float hd,
    const glm::vec3& col, unsigned int tex = 0, float texScale = 1.f)
{
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { cx, cy, cz }),
        { hw * 2.f, hh * 2.f, hd * 2.f });
    drawCube(sh, m, col, tex, texScale);
}

static void emitBox(unsigned int sh, float cx, float cy, float cz, float hw, float hh, float hd,
    const glm::vec3& col, const glm::vec3& emit)
{
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { cx, cy, cz }),
        { hw * 2.f, hh * 2.f, hd * 2.f });
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(col));
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(emit));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(glm::vec3(0.f)));
}

static void framePicture(unsigned int sh, float cx, float cy, float cz, float hw, float hh,
    const glm::vec3& frameCol, const glm::vec3& artCol)
{
    box(sh, cx, cy, cz, hw + 0.08f, hh + 0.08f, 0.04f, frameCol, texWood, 2.f);
    box(sh, cx, cy, cz + 0.02f, hw, hh, 0.02f, artCol);
}

static void drawCeilingTiles(unsigned int sh)
{
    box(sh, 0.f, CEIL_Y + 0.03f, -5.f, 10.f, 0.03f, 10.f, C_CEIL, texConcrete, 6.f);

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float tx = -7.5f + col * 5.0f;
            float tz = -13.5f + row * 5.0f;
            box(sh, tx, CEIL_Y - 0.01f, tz, 1.60f, 0.02f, 1.60f, C_TRIM);
        }
    }

    for (int c : {0, 2}) {
        for (int r : {1, 3}) {
            float tx = -5.0f + c * 5.0f;
            float tz = -13.5f + r * 5.0f;
            emitBox(sh, tx, CEIL_Y - 0.01f, tz, 1.35f, 0.02f, 1.70f, C_LAMP, C_GLOW_WARM * 0.6f);
        }
    }
}

static void drawWalls(unsigned int sh)
{
    box(sh, 0.f, RY + ROOM_H * 0.5f, -15.f, 10.f, ROOM_H * 0.5f, 0.1f, C_WALL, texConcrete, 4.f);
    box(sh, -10.f, RY + ROOM_H * 0.5f, -5.f, 0.1f, ROOM_H * 0.5f, 10.f, C_WALL, texConcrete, 4.f);
    box(sh, 10.f, RY + ROOM_H * 0.5f, -5.f, 0.1f, ROOM_H * 0.5f, 10.f, C_WALL, texConcrete, 4.f);
    box(sh, -6.f, RY + ROOM_H * 0.5f, 5.f, 4.f, ROOM_H * 0.5f, 0.1f, C_WALL, texConcrete, 2.f);
    box(sh, 6.f, RY + ROOM_H * 0.5f, 5.f, 4.f, ROOM_H * 0.5f, 0.1f, C_WALL, texConcrete, 2.f);
    box(sh, 0.f, RY + ROOM_H - 0.7f, 5.f, 2.f, 0.7f, 0.1f, C_WALL, texConcrete, 1.f);
}

static void drawWallPanels(unsigned int sh)
{
    for (int side : {-1, 1}) {
        float wx = side * 9.95f;
        for (int i = 0; i < 2; ++i) {
            float pz = -8.f + i * 6.f;
            box(sh, wx, RY + 2.5f, pz, 0.03f, 1.8f, 2.2f, C_WALL_ACCENT);
            box(sh, wx, RY + 2.5f, pz, 0.04f, 1.75f, 2.15f, C_TRIM);
        }
    }
}

static void drawWallArt(unsigned int sh)
{
    framePicture(sh, -5.f, RY + 4.2f, -14.95f, 1.2f, 0.8f, C_WOOD_DK, C_ART1);
    framePicture(sh, 5.f, RY + 4.2f, -14.95f, 1.0f, 1.0f, C_METAL, C_ART2);
    framePicture(sh, 0.f, RY + 4.5f, 4.95f, 1.5f, 0.6f, C_WOOD_MD, glm::vec3(0.50f, 0.65f, 0.70f));
}

static void drawFloatingShelf(unsigned int sh, float sx, float sy, float sz)
{
    box(sh, sx, sy, sz, 0.8f, 0.04f, 0.25f, C_WOOD_DK, texWood, 1.f);
    box(sh, sx, sy - 0.08f, sz, 0.75f, 0.04f, 0.02f, C_WOOD_MD);
    box(sh, sx, sy - 0.18f, sz, 0.70f, 0.04f, 0.02f, C_WOOD_MD);
    glm::mat4 vase = glm::scale(
        glm::translate(glm::mat4(1.f), { sx + 0.3f, sy + 0.25f, sz }),
        { 0.08f, 0.20f, 0.08f });
    drawSphere(sh, vase, C_CUSHION2);
    box(sh, sx - 0.4f, sy + 0.12f, sz, 0.12f, 0.08f, 0.18f, glm::vec3(0.70f, 0.65f, 0.55f));
}

static void drawFloor(unsigned int sh)
{
    box(sh, 0.f, RY, -5.f, 10.f, 0.05f, 10.f, C_FLOOR, texMarble, 5.f);

    for (float z = -13.f; z <= 3.f; z += 2.f) {
        box(sh, -8.f, RY + 0.051f, z, 1.5f, 0.001f, 0.01f, C_WOOD_DK);
        box(sh, 8.f, RY + 0.051f, z, 1.5f, 0.001f, 0.01f, C_WOOD_DK);
    }

    for (float x = -8.f; x <= 8.f; x += 2.f) {
        box(sh, x, RY + 0.051f, -3.f, 0.01f, 0.001f, 1.5f, C_WOOD_DK);
    }
}

static void drawTVMediaConsole(unsigned int sh)
{
    const float TCX = 0.f, TCY = RY + 0.05f, TCZ = -13.2f;

    box(sh, TCX, TCY + 0.45f, TCZ, 4.5f, 0.08f, 0.55f, C_WOOD_DK, texWood, 3.f);
    box(sh, TCX, TCY + 0.22f, TCZ, 4.4f, 0.18f, 0.50f, C_WOOD_DK, texWood, 3.f);

    for (int i = -1; i <= 1; ++i) {
        float dx = i * 1.4f;
        box(sh, TCX + dx, TCY + 0.12f, TCZ, 0.55f, 0.12f, 0.42f, C_SHADOW);
        emitBox(sh, TCX + dx, TCY + 0.08f, TCZ, 0.50f, 0.02f, 0.38f, C_METAL, C_GLOW_COOL * 0.15f);
    }

    for (int i = -1; i <= 1; ++i) {
        float dx = i * 1.4f;
        box(sh, TCX + dx, TCY + 0.52f, TCZ - 0.2f, 0.06f, 0.35f, 0.06f, C_METAL);
        box(sh, TCX + dx, TCY + 0.52f, TCZ + 0.2f, 0.06f, 0.35f, 0.06f, C_METAL);
    }

    box(sh, TCX, TCY + 0.35f, TCZ + 0.6f, 0.25f, 0.35f, 0.08f, C_METAL);
    glm::mat4 speaker = glm::scale(
        glm::translate(glm::mat4(1.f), { TCX + 3.8f, TCY + 0.35f, TCZ + 0.4f }),
        { 0.15f, 0.25f, 0.10f });
    drawCylinder(sh, speaker, C_SHADOW);
}

static void drawTV(unsigned int sh)
{
    float t = (float)glfwGetTime();
    float pulse = 0.5f + 0.5f * sinf(t * 1.7f);
    float sweep = fmodf(t * 1.8f, 2.4f) - 1.2f; // -1.2 .. +1.2 across screen height

    glm::vec3 dynBase(
        0.10f + 0.20f * (0.5f + 0.5f * sinf(t * 0.9f)),
        0.18f + 0.35f * (0.5f + 0.5f * sinf(t * 1.1f + 1.6f)),
        0.35f + 0.45f * (0.5f + 0.5f * sinf(t * 1.3f + 3.1f)));

    box(sh, 0.f, RY + 3.4f, -14.75f, 4.0f, 1.8f, 0.08f, C_WALL_ACCENT);
    box(sh, 0.f, RY + 3.4f, -14.68f, 3.8f, 1.5f, 0.06f, C_TV_BODY);
    box(sh, 0.f, RY + 3.42f, -14.62f, 3.5f, 1.25f, 0.015f,
        C_TV_SCRN + dynBase * 0.18f);

    // Animated full-screen glow tint
    emitBox(sh, 0.f, RY + 3.42f, -14.61f,
        3.45f, 1.22f, 0.01f,
        dynBase * 0.35f,
        dynBase * (0.18f + 0.20f * pulse));

    // Bottom ticker strip
    emitBox(sh, 0.f, RY + 2.12f, -14.61f,
        3.5f, 0.04f, 0.01f,
        glm::vec3(0.15f, 0.30f, 0.70f),
        glm::vec3(0.05f, 0.10f, 0.25f));

    // Vertical bars shimmer
    for (float i = -1.5f; i <= 1.5f; i += 0.5f) {
        float k = 0.3f + 0.7f * (0.5f + 0.5f * sinf(t * 2.0f + i * 4.0f));
        emitBox(sh, i, RY + 3.42f, -14.61f,
            0.015f, 1.2f, 0.01f,
            glm::vec3(0.02f, 0.02f, 0.05f),
            glm::vec3(0.01f, 0.01f, 0.02f) * k);
    }

    // Moving scanline
    emitBox(sh,
        0.f, RY + 3.42f + sweep, -14.61f,
        3.45f, 0.03f, 0.01f,
        glm::vec3(0.75f, 0.85f, 1.0f),
        glm::vec3(0.25f, 0.35f, 0.50f));

    box(sh, 0.f, RY + 3.38f, -14.55f, 0.35f, 0.12f, 0.20f, C_METAL);

    glm::mat4 led = glm::scale(
        glm::translate(glm::mat4(1.f), { 3.45f, RY + 4.75f, -14.60f }),
        { 0.05f, 0.05f, 0.05f });
    drawSphere(sh, led, glm::vec3(0.f, 1.f, 0.2f));

    box(sh, -3.2f, RY + 3.0f, -14.5f, 0.6f, 0.5f, 0.15f, C_SHADOW);
    box(sh, 3.2f, RY + 3.0f, -14.5f, 0.6f, 0.5f, 0.15f, C_SHADOW);
    glm::mat4 spk1 = glm::scale(
        glm::translate(glm::mat4(1.f), { -3.2f, RY + 3.0f, -14.35f }),
        { 0.15f, 0.12f, 0.10f });
    drawCylinder(sh, spk1, C_SHADOW);
    glm::mat4 spk2 = glm::scale(
        glm::translate(glm::mat4(1.f), { 3.2f, RY + 3.0f, -14.35f }),
        { 0.15f, 0.12f, 0.10f });
    drawCylinder(sh, spk2, C_SHADOW);
}

static void drawSofa(unsigned int sh)
{
    const float BASE_Y = RY + 0.05f;
    const float S_HALF_W = 2.60f;
    const float S_HALF_D = 0.90f;
    const float S_PLAT_HH = 0.25f;
    const float S_CUSH_HH = 0.18f;
    const float S_BACK_HH = 0.55f;
    const float S_BACK_HD = 0.16f;
    const float S_ARM_HZ = 0.16f;
    const float S_ARM_HH = 0.60f;
    const float SZ = -4.5f;

    auto drawSofaBase = [&](float cx, float backSign) {
        box(sh, cx, BASE_Y + S_PLAT_HH, SZ, S_HALF_D, S_PLAT_HH, S_HALF_W, C_WOOD_DK, texWood, 2.f);

        const float cushZ[3] = { SZ - 1.60f, SZ, SZ + 1.60f };
        for (float cz : cushZ) {
            box(sh, cx, BASE_Y + S_PLAT_HH * 2.f + S_CUSH_HH, cz, S_HALF_D - 0.06f, S_CUSH_HH, 0.72f, C_CUSHION);
        }

        float backX = cx + backSign * (S_HALF_D - S_BACK_HD);
        box(sh, backX, BASE_Y + S_PLAT_HH * 2.f + S_BACK_HH, SZ, S_BACK_HD, S_BACK_HH, S_HALF_W, C_WOOD_DK, texWood, 1.f);
        box(sh, backX - backSign * 0.08f, BASE_Y + S_PLAT_HH * 2.f + S_BACK_HH, SZ, S_BACK_HD * 0.50f, S_BACK_HH - 0.05f, S_HALF_W - 0.08f, C_WOOD_LT);

        for (int side : {-1, +1}) {
            float az = SZ + side * (S_HALF_W + S_ARM_HZ);
            box(sh, cx, BASE_Y + S_ARM_HH, az, S_HALF_D, S_ARM_HH, S_ARM_HZ, C_WOOD_DK, texWood, 1.f);
            box(sh, cx, BASE_Y + S_ARM_HH * 1.8f, az, S_HALF_D - 0.08f, S_ARM_HH * 0.3f, S_ARM_HZ * 0.8f, C_CUSHION2);
        }

        for (int lx : {-1, +1}) for (int lz : {-1, +1}) {
            box(sh, cx + lx * (S_HALF_D - 0.10f), BASE_Y + 0.10f, SZ + lz * (S_HALF_W - 0.15f), 0.06f, 0.10f, 0.06f, C_METAL);
        }

        for (int i = 0; i < 3; ++i) {
            float cz = SZ - 1.30f + i * 1.30f;
            box(sh, cx + backSign * 0.05f, BASE_Y + S_PLAT_HH * 2.f + S_BACK_HH * 2.f + 0.08f, cz, S_BACK_HD * 0.7f, 0.14f, 0.30f, C_CUSHION2);
        }
        };

    drawSofaBase(-8.0f, -1.f);
    drawSofaBase(8.0f, 1.f);
}

static void drawSideTable(unsigned int sh, float sx, float sz)
{
    const float TY = RY + 0.05f;

    box(sh, sx, TY + 0.55f, sz, 0.40f, 0.05f, 0.40f, C_WOOD_MD, texWood, 1.f);
    box(sh, sx, TY + 0.28f, sz, 0.35f, 0.03f, 0.35f, C_WOOD_DK);

    for (int lx : {-1, 1}) for (int lz : {-1, 1}) {
        box(sh, sx + lx * 0.32f, TY + 0.30f, sz + lz * 0.32f, 0.04f, 0.28f, 0.04f, C_METAL);
    }

    glm::mat4 lampBase = glm::scale(
        glm::translate(glm::mat4(1.f), { sx, TY + 0.60f, sz }),
        { 0.12f, 0.05f, 0.12f });
    drawCylinder(sh, lampBase, C_METAL);
    box(sh, sx, TY + 0.85f, sz, 0.04f, 0.25f, 0.04f, C_METAL);
    box(sh, sx, TY + 1.20f, sz, 0.20f, 0.15f, 0.20f, C_WHITE);
    emitBox(sh, sx, TY + 1.18f, sz, 0.18f, 0.12f, 0.18f, C_LAMP, C_GLOW_WARM * 0.5f);
}

static void drawTeaTable(unsigned int sh)
{
    const float TX = 0.f, TZ = -7.0f, TY = RY + 0.05f;

    box(sh, TX, TY + 0.50f, TZ, 1.80f, 0.06f, 0.80f, C_WOOD_MD, texWood, 1.f);
    box(sh, TX, TY + 0.22f, TZ, 1.60f, 0.04f, 0.68f, C_WOOD_DK);

    for (int lx : {-1, 1}) for (int lz : {-1, 1}) {
        box(sh, TX + lx * 1.60f, TY + 0.26f, TZ + lz * 0.65f, 0.06f, 0.26f, 0.06f, C_METAL);
    }

    glm::mat4 bowl = glm::scale(
        glm::translate(glm::mat4(1.f), { TX + 0.4f, TY + 0.62f, TZ }),
        { 0.22f, 0.12f, 0.22f });
    drawSphere(sh, bowl, C_GLASS);
    emitBox(sh, TX + 0.4f, TY + 0.58f, TZ, 0.18f, 0.02f, 0.18f, C_GLOW_COOL * 0.3f, C_GLOW_COOL * 0.15f);

    box(sh, TX - 0.6f, TY + 0.58f, TZ + 0.20f, 0.18f, 0.025f, 0.07f, glm::vec3(0.08f, 0.08f, 0.10f));
    box(sh, TX - 0.35f, TY + 0.58f, TZ - 0.25f, 0.32f, 0.030f, 0.22f, glm::vec3(0.65f, 0.30f, 0.15f));
    box(sh, TX - 0.35f, TY + 0.62f, TZ - 0.25f, 0.30f, 0.025f, 0.20f, glm::vec3(0.20f, 0.40f, 0.60f));
    box(sh, TX + 0.1f, TY + 0.60f, TZ - 0.28f, 0.10f, 0.04f, 0.08f, C_GOLD);

    glm::mat4 candle = glm::scale(
        glm::translate(glm::mat4(1.f), { TX - 0.8f, TY + 0.68f, TZ - 0.1f }),
        { 0.04f, 0.08f, 0.04f });
    drawCylinder(sh, candle, C_WHITE);
    emitBox(sh, TX - 0.8f, TY + 0.80f, TZ - 0.1f, 0.02f, 0.02f, 0.02f, C_GLOW_WARM, C_GLOW_WARM * 0.8f);
}

static void drawPlant(unsigned int sh, float px, float pz)
{
    const float PY = RY + 0.05f;

    box(sh, px, PY + 0.35f, pz, 0.22f, 0.32f, 0.22f, C_POT);
    box(sh, px, PY + 0.68f, pz, 0.26f, 0.04f, 0.26f, C_POT);

    for (int i = 0; i < 5; ++i) {
        float angle = i * 72.f;
        float ox = cosf(angle * 3.14159f / 180.f) * 0.12f;
        float oz = sinf(angle * 3.14159f / 180.f) * 0.12f;
        glm::mat4 leaf = glm::scale(
            glm::translate(glm::mat4(1.f), { px + ox, PY + 0.85f, pz + oz }),
            { 0.12f, 0.20f, 0.08f });
        drawSphere(sh, leaf, C_PLANT);
    }
    glm::mat4 top = glm::scale(
        glm::translate(glm::mat4(1.f), { px, PY + 0.90f, pz }),
        { 0.10f, 0.15f, 0.10f });
    drawSphere(sh, top, C_PLANT);
}

static void drawDoor(unsigned int sh)
{
}

static void drawStaircase(unsigned int sh)
{
}

static void drawWallLight(unsigned int sh)
{
    box(sh, -8.6f, RY + 5.0f, -14.5f, 0.40f, 0.25f, 0.10f, C_METAL);
    box(sh, -8.6f, RY + 4.65f, -14.45f, 0.35f, 0.08f, 0.12f, C_METAL);

    for (int arm = 0; arm < 2; ++arm) {
        float ax = -8.6f + (arm == 0 ? -0.25f : 0.25f);
        box(sh, ax, RY + 4.80f, -14.4f, 0.04f, 0.20f, 0.06f, C_GOLD);
    }

    glm::mat4 bh = glm::scale(
        glm::translate(glm::mat4(1.f), { -8.6f, RY + 4.55f, -14.38f }),
        { 0.18f, 0.15f, 0.12f });
    drawSphere(sh, bh, C_LAMP);
    emitBox(sh, -8.6f, RY + 4.55f, -14.35f, 0.15f, 0.12f, 0.08f, C_GLOW_WARM, C_GLOW_WARM * 0.4f);

    box(sh, 8.6f, RY + 5.0f, -14.5f, 0.40f, 0.25f, 0.10f, C_METAL);
    box(sh, 8.6f, RY + 4.65f, -14.45f, 0.35f, 0.08f, 0.12f, C_METAL);
    for (int arm = 0; arm < 2; ++arm) {
        float ax = 8.6f + (arm == 0 ? -0.25f : 0.25f);
        box(sh, ax, RY + 4.80f, -14.4f, 0.04f, 0.20f, 0.06f, C_GOLD);
    }
    glm::mat4 bh2 = glm::scale(
        glm::translate(glm::mat4(1.f), { 8.6f, RY + 4.55f, -14.38f }),
        { 0.18f, 0.15f, 0.12f });
    drawSphere(sh, bh2, C_LAMP);
    emitBox(sh, 8.6f, RY + 4.55f, -14.35f, 0.15f, 0.12f, 0.08f, C_GLOW_WARM, C_GLOW_WARM * 0.4f);
}

static void drawSkirting(unsigned int sh)
{
    const glm::vec3 SK(0.18f, 0.12f, 0.08f);
    const float SH2 = 0.08f, SD2 = 0.06f;
    const float sy = RY + SH2;

    box(sh, 0.f, sy, -14.92f, 9.8f, SH2, SD2, SK);
    box(sh, -9.92f, sy, -5.f, SD2, SH2, 9.8f, SK);
    box(sh, 9.92f, sy, -5.f, SD2, SH2, 9.8f, SK);
    box(sh, -6.0f, sy, 4.92f, 3.8f, SH2, SD2, SK);
    box(sh, 6.0f, sy, 4.92f, 3.8f, SH2, SD2, SK);
}

static void drawCrownMolding(unsigned int sh)
{
    const float CY = CEIL_Y - 0.08f;

    box(sh, 0.f, CY, -14.92f, 9.8f, 0.06f, 0.08f, C_TRIM);
    box(sh, -9.92f, CY, -5.f, 0.08f, 0.06f, 9.8f, C_TRIM);
    box(sh, 9.92f, CY, -5.f, 0.08f, 0.06f, 9.8f, C_TRIM);
    box(sh, -6.0f, CY, 4.92f, 3.8f, 0.06f, 0.08f, C_TRIM);
    box(sh, 6.0f, CY, 4.92f, 3.8f, 0.06f, 0.08f, C_TRIM);
}

static void drawRug(unsigned int sh)
{
    box(sh, 0.f, RY + 0.052f, -6.f, 3.5f, 0.01f, 2.5f, C_CUSHION2);
    box(sh, 0.f, RY + 0.053f, -6.f, 3.3f, 0.005f, 2.3f, C_CUSHION);
    box(sh, 0.f, RY + 0.054f, -6.f, 3.0f, 0.003f, 2.0f, C_WOOD_LT);

    for (int i = -2; i <= 2; ++i) {
        box(sh, i * 0.6f, RY + 0.055f, -6.f, 0.02f, 0.001f, 1.8f, C_WOOD_DK);
    }
    for (int i = -1; i <= 1; ++i) {
        box(sh, 0.f, RY + 0.055f, -6.f + i * 0.8f, 2.6f, 0.001f, 0.02f, C_WOOD_DK);
    }
}

void drawTVRoom(unsigned int sh)
{
    drawFloor(sh);
    drawRug(sh);
    drawWalls(sh);
    drawWallPanels(sh);
    drawSkirting(sh);
    drawCrownMolding(sh);
    drawCeilingTiles(sh);
    drawTVMediaConsole(sh);
    drawTV(sh);
    drawWallArt(sh);
    drawFloatingShelf(sh, -7.f, RY + 4.5f, -9.f);
    drawFloatingShelf(sh, 7.f, RY + 4.5f, -9.f);
    drawSofa(sh);
    drawSideTable(sh, -5.5f, -7.5f);
    drawSideTable(sh, 5.5f, -7.5f);
    drawTeaTable(sh);
    drawPlant(sh, -6.f, -10.f);
    drawPlant(sh, 6.f, -10.f);
    drawPlant(sh, -8.f, 2.f);
    drawPlant(sh, 8.f, 2.f);
    drawWallLight(sh);
}