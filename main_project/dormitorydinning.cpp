// dormitory_dining.cpp  —  University Dormitory Dining Hall (Fixed)
//
// CAMERA FIX:
//   Camera placed at centre of room (0, 1.7, 0), looking forward (-Z).
//   All duplicate declarations removed.
//   dhSetTopViewCamera now sets position ONCE with no contradictions.
//
// ROOM CONSTANTS
//   X : -16 ... +16  (width  32)
//   Z : -10 ... +10  (depth  20)
//   Y :   0 ...  4.2 (ceiling)
// -------------------------------------------------------------------------

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "globals.h"
#include "room.h"
#include "draw_helpers.h"
#include "utils.h"

#include <cmath>
#include <vector>
#include <fstream>

// ==========================================================================
//  MODULE GLOBALS
// ==========================================================================
static float dhFanAngle = 0.0f;
static bool  dhLightOn = true;
static unsigned int dh_sh = 0;
static bool  dhCameraSet = false;   // ← only ONE declaration

static const float DH_FAN_DEG_PER_SEC = 120.f;

// ==========================================================================
//  ROOM CONSTANTS
// ==========================================================================
static const float DX1 = -16.f, DX2 = 16.f;
static const float DZ1 = -10.f, DZ2 = 10.f;
static const float DY = 4.2f;

// ==========================================================================
//  BEZIER BASIN GLOBALS
// ==========================================================================
static unsigned int dhBasinVAO = 0;
static unsigned int dhBasinIdxCnt = 0;
static const int DH_NT = 40;
static const int DH_NTHETA = 36;

// ==========================================================================
//  HELPERS
// ==========================================================================
static void dhRenderCube() {
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void dhCol(float r, float g, float b) {
    glUniform3f(glGetUniformLocation(dh_sh, "objectColor"), r, g, b);
    glUniform1i(glGetUniformLocation(dh_sh, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(dh_sh, "textureMode"), 0);
}

static void dhSetModel(glm::mat4 m) {
    glUniformMatrix4fv(glGetUniformLocation(dh_sh, "model"),
        1, GL_FALSE, glm::value_ptr(m));
    glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(m)));
    int nl = glGetUniformLocation(dh_sh, "normalMatrix");
    if (nl >= 0) glUniformMatrix3fv(nl, 1, GL_FALSE, glm::value_ptr(nm));
}

static void DB(float cx, float cy, float cz,
    float sx, float sy, float sz)
{
    glm::mat4 m = glm::translate(glm::mat4(1.f), { cx, cy, cz });
    m = glm::scale(m, { sx, sy, sz });
    dhSetModel(m);
    dhRenderCube();
}

// ==========================================================================
//  CAMERA — centred in room, eye-level, looking toward back wall (-Z)
//
//  Position : (0, 1.7, 4)   — centre X, eye height, slightly in front of door
//  Front    : (0, 0, -1)    — looking straight toward back wall
//  Yaw      : -90°          — standard "look along -Z" yaw
//  Pitch    :   0°          — level gaze
// ==========================================================================
static void dhSetTopViewCamera() {
    camera.position = glm::vec3(0.f, 1.7f, 4.f);
    camera.front = glm::vec3(0.f, 0.f, -1.f);
    camera.up = glm::vec3(0.f, 1.f, 0.f);
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.roll = 0.0f;
    camera.orbitMode = false;
    camera.birdsEyeMode = false;
    updateCameraVectors();
}

// ==========================================================================
//  BEZIER BASIN
// ==========================================================================
static long long dhNCR(int n, int r) {
    if (r == 0 || r == n) return 1LL;
    if (r > n - r) r = n - r;
    long long res = 1;
    for (int i = 0; i < r; ++i) { res *= (n - i); res /= (i + 1); }
    return res;
}

static void dhBezierCurve(double t, float xy[2],
    const float* ctrlPts, int L)
{
    t = (t > 1.0) ? 1.0 : t;
    double xA = 0.0, yA = 0.0;
    for (int i = 0; i <= L; ++i) {
        long long ncr = dhNCR(L, i);
        double c = pow(1.0 - t, double(L - i)) * pow(t, double(i)) * double(ncr);
        xA += c * ctrlPts[i * 3];
        yA += c * ctrlPts[i * 3 + 1];
    }
    xy[0] = float(xA); xy[1] = float(yA);
}

static unsigned int dhHollowBezier(const float* ctrlPts, int L,
    unsigned int& outIndexCount)
{
    std::vector<float>        coordinates, normals, texCoords, vertices;
    std::vector<unsigned int> indices;

    const double dTheta = (2.0 * 3.14159265358979) / double(DH_NTHETA);
    const double dt = 1.0 / double(DH_NT);
    double t = 0.0;

    for (int i = 0; i <= DH_NT; ++i) {
        float xy[2];
        dhBezierCurve(t, xy, ctrlPts, L);
        float r = xy[0], y = xy[1];
        float li = (r > 1e-6f) ? (1.f / r) : 0.f;
        double theta = 0.0;
        for (int j = 0; j <= DH_NTHETA; ++j) {
            float cs = float(cos(theta)), sn = float(sin(theta));
            float x = r * sn, z = r * cs;
            coordinates.push_back(x); coordinates.push_back(y); coordinates.push_back(z);
            normals.push_back(-x * li); normals.push_back(0.f); normals.push_back(-z * li);
            texCoords.push_back(float(j) / float(DH_NTHETA));
            texCoords.push_back(float(t));
            theta += dTheta;
        }
        t += dt;
    }

    for (int i = 0; i < DH_NT; ++i) {
        int k1 = i * (DH_NTHETA + 1), k2 = (i + 1) * (DH_NTHETA + 1);
        for (int j = 0; j < DH_NTHETA; ++j, ++k1, ++k2) {
            indices.push_back(k1);     indices.push_back(k2);     indices.push_back(k1 + 1);
            indices.push_back(k1 + 1); indices.push_back(k2);     indices.push_back(k2 + 1);
        }
    }

    size_t cnt = coordinates.size();
    for (size_t i = 0; i < cnt; i += 3) {
        int vi = int(i / 3);
        vertices.push_back(coordinates[i]);   vertices.push_back(coordinates[i + 1]); vertices.push_back(coordinates[i + 2]);
        vertices.push_back(normals[i]);        vertices.push_back(normals[i + 1]);     vertices.push_back(normals[i + 2]);
        vertices.push_back(texCoords[vi * 2]); vertices.push_back(texCoords[vi * 2 + 1]);
    }

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &ebo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    int stride = 32;
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    outIndexCount = (unsigned int)indices.size();
    return vao;
}

static void dhInitBasin() {
    static const float defaultPts[] = {
        0.06f, 0.00f, 0.0f,
        0.22f, 0.03f, 0.0f,
        0.42f, 0.10f, 0.0f,
        0.52f, 0.22f, 0.0f,
        0.56f, 0.32f, 0.0f
    };
    dhBasinVAO = dhHollowBezier(defaultPts, 4, dhBasinIdxCnt);
}

static void dhDrawBasinMesh(float wx, float rimY, float wz, float scale) {
    if (dhBasinVAO == 0) dhInitBasin();
    dhCol(0.98f, 0.97f, 0.96f);
    glm::mat4 m = glm::translate(glm::mat4(1.f), { wx, rimY, wz });
    m = glm::scale(m, { scale, scale, scale });
    dhSetModel(m);
    glBindVertexArray(dhBasinVAO);
    glDrawElements(GL_TRIANGLES, dhBasinIdxCnt, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ==========================================================================
//  LIGHTING
// ==========================================================================
static void dhLighting() {
    glm::vec3 amb = dhLightOn ? glm::vec3(0.55f, 0.54f, 0.52f)
        : glm::vec3(0.05f, 0.05f, 0.05f);
    glUniform3fv(glGetUniformLocation(dh_sh, "ambientColor"), 1, glm::value_ptr(amb));
    glUniform3f(glGetUniformLocation(dh_sh, "lightPos"), 0.f, 6.f, 0.f);
    glUniform3f(glGetUniformLocation(dh_sh, "lightColor"), 1.0f, 0.97f, 0.92f);
    glUniform1f(glGetUniformLocation(dh_sh, "lightIntensity"), dhLightOn ? 2.0f : 0.f);
    glUniform3f(glGetUniformLocation(dh_sh, "lightPos2"), DX1 + 1.5f, 3.0f, 0.f);
    glUniform3f(glGetUniformLocation(dh_sh, "lightColor2"), 1.0f, 0.98f, 0.94f);
    glUniform1f(glGetUniformLocation(dh_sh, "lightIntensity2"), dhLightOn ? 1.2f : 0.f);
}

// ==========================================================================
//  ROOM SHELL
// ==========================================================================
static void dhShell() {
    float W = DX2 - DX1, D = DZ2 - DZ1;
    float mx = 0.f, mz = 0.f;

    // Floor tiles (2x2 checker)
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 10; j++) {
            bool ev = (i + j) % 2 == 0;
            dhCol(ev ? 0.94f : 0.86f, ev ? 0.90f : 0.82f, ev ? 0.84f : 0.74f);
            DB(DX1 + i * 2.f + 1.f, 0.01f, DZ1 + j * 2.f + 1.f, 1.98f, 0.04f, 1.98f);
        }
    // Floor base
    dhCol(0.72f, 0.68f, 0.62f);
    DB(mx, 0.0f, mz, W, 0.02f, D);

    // Ceiling
    dhCol(0.96f, 0.96f, 0.95f);
    DB(mx, DY, mz, W, 0.10f, D);

    // Ceiling grid lines
    dhCol(0.82f, 0.82f, 0.80f);
    for (int i = 0; i <= 16; i++) DB(DX1 + i * 2.f, DY - 0.01f, mz, 0.04f, 0.04f, D);
    for (int j = 0; j <= 10; j++) DB(mx, DY - 0.01f, DZ1 + j * 2.f, W, 0.04f, 0.04f);

    // Back wall (Z = DZ1 = -10)
    dhCol(0.88f, 0.86f, 0.82f);
    DB(mx, DY * 0.5f, DZ1, W, DY, 0.12f);
    dhCol(0.76f, 0.72f, 0.66f);
    DB(mx, 1.0f, DZ1 + 0.06f, W, 2.0f, 0.08f);

    // Left wall (X = DX1 = -16)
    dhCol(0.90f, 0.88f, 0.85f);
    DB(DX1, DY * 0.5f, mz, 0.12f, DY, D);
    dhCol(0.76f, 0.72f, 0.66f);
    DB(DX1 + 0.06f, 1.0f, mz, 0.08f, 2.0f, D);

    // Right wall (X = DX2 = +16)
    dhCol(0.90f, 0.89f, 0.87f);
    DB(DX2, DY * 0.5f, mz, 0.12f, DY, D);

    // Front wall (Z = DZ2 = +10)
    dhCol(0.91f, 0.90f, 0.88f);
    DB(mx, DY * 0.5f, DZ2, W, DY, 0.12f);

    // Skirting boards
    dhCol(0.60f, 0.56f, 0.50f);
    DB(mx, 0.15f, DZ1 + 0.06f, W, 0.25f, 0.07f);
    DB(mx, 0.15f, DZ2 - 0.06f, W, 0.25f, 0.07f);
    DB(DX1 + 0.06f, 0.15f, mz, 0.07f, 0.25f, D);
    DB(DX2 - 0.06f, 0.15f, mz, 0.07f, 0.25f, D);
}

// ==========================================================================
//  RIGHT WALL: WINDOWS
// ==========================================================================
static void dhRightWindows() {
    float rx = DX2;
    dhCol(0.82f, 0.82f, 0.80f);
    for (float z = DZ1; z <= DZ2 + 0.1f; z += 5.f)
        DB(rx - 0.18f, DY * 0.5f, z, 0.28f, DY + 0.1f, 0.22f);
    for (int p = 0; p < 4; p++) {
        float zc = DZ1 + p * 5.f + 2.5f;
        dhCol(0.62f, 0.80f, 0.94f);
        DB(rx + 0.02f, DY * 0.5f + 0.2f, zc, 0.06f, DY - 0.5f, 4.70f);
        dhCol(0.82f, 0.92f, 0.96f);
        DB(rx - 0.10f, DY * 0.5f, zc, 0.05f, DY - 0.22f, 4.68f);
        dhCol(0.80f, 0.80f, 0.78f);
        DB(rx - 0.12f, 1.50f, zc, 0.07f, 0.09f, 4.68f);
        DB(rx - 0.12f, 2.80f, zc, 0.07f, 0.09f, 4.68f);
        dhCol(0.78f, 0.76f, 0.72f);
        DB(rx - 0.30f, 0.40f, zc, 0.46f, 0.07f, 4.68f);
    }
}

// ==========================================================================
//  FRONT WALL: ENTRANCE
// ==========================================================================
static void dhEntrance() {
    float fz = DZ2, ex = -1.0f;
    dhCol(0.56f, 0.56f, 0.56f);
    DB(ex, DY * 0.5f, fz - 0.06f, 2.40f, DY, 0.10f);
    DB(ex - 1.15f, 1.20f, fz - 0.05f, 0.12f, 2.40f, 0.08f);
    DB(ex + 1.15f, 1.20f, fz - 0.05f, 0.12f, 2.40f, 0.08f);
    dhCol(0.78f, 0.90f, 0.96f);
    DB(ex - 0.56f, 1.20f, fz - 0.04f, 0.98f, 2.40f, 0.06f);
    DB(ex + 0.56f, 1.20f, fz - 0.04f, 0.98f, 2.40f, 0.06f);
    dhCol(0.76f, 0.72f, 0.64f);
    DB(ex - 0.10f, 1.20f, fz - 0.02f, 0.08f, 0.06f, 0.06f);
    DB(ex + 0.10f, 1.20f, fz - 0.02f, 0.08f, 0.06f, 0.06f);
    dhCol(0.80f, 0.90f, 0.96f);
    DB(ex, 2.65f, fz - 0.04f, 2.20f, 0.70f, 0.06f);
}

// ==========================================================================
//  BACK WALL: SERVING COUNTER / FOOD BOOTH
// ==========================================================================
static void dhServingCounter() {
    float cz = DZ1;
    dhCol(0.58f, 0.58f, 0.60f);
    DB(0.f, 0.56f, cz + 0.56f, 26.f, 1.12f, 1.00f);
    dhCol(0.74f, 0.74f, 0.76f);
    DB(0.f, 1.13f, cz + 0.56f, 26.f, 0.05f, 1.02f);

    // Sneeze guard
    dhCol(0.84f, 0.93f, 0.97f);
    DB(0.f, 1.72f, cz + 0.28f, 26.f, 1.20f, 0.04f);
    dhCol(0.60f, 0.60f, 0.62f);
    for (float x = -13.f; x <= 13.1f; x += 4.f)
        DB(x, 1.72f, cz + 0.28f, 0.06f, 1.22f, 0.06f);

    // Hot-food trays
    static const float foodCol[3][3] = {
        {0.86f,0.56f,0.24f},{0.50f,0.66f,0.34f},{0.70f,0.38f,0.22f}
    };
    for (int t = 0; t < 3; t++) {
        float tx = -10.f + t * 3.2f;
        dhCol(0.52f, 0.52f, 0.54f); DB(tx, 1.18f, cz + 0.50f, 2.80f, 0.08f, 0.80f);
        dhCol(foodCol[t][0], foodCol[t][1], foodCol[t][2]);
        DB(tx, 1.22f, cz + 0.50f, 2.60f, 0.04f, 0.70f);
    }

    // Salad bar
    dhCol(0.78f, 0.88f, 0.94f);
    DB(2.5f, 1.16f, cz + 0.52f, 5.00f, 0.06f, 0.82f);
    float saladX[] = { 1.0f, 2.5f, 4.0f };
    float saladC[3][3] = {
        {0.38f,0.64f,0.30f},{0.90f,0.30f,0.24f},{0.96f,0.88f,0.40f}
    };
    for (int s = 0; s < 3; s++) {
        dhCol(0.88f, 0.88f, 0.88f); DB(saladX[s], 1.22f, cz + 0.52f, 0.76f, 0.08f, 0.76f);
        dhCol(saladC[s][0], saladC[s][1], saladC[s][2]);
        DB(saladX[s], 1.28f, cz + 0.52f, 0.64f, 0.06f, 0.64f);
    }

    // Bread / dessert
    dhCol(0.82f, 0.70f, 0.50f); DB(8.f, 1.16f, cz + 0.50f, 4.50f, 0.06f, 0.80f);
    dhCol(0.72f, 0.52f, 0.28f);
    DB(7.0f, 1.24f, cz + 0.52f, 1.10f, 0.18f, 0.50f);
    DB(8.6f, 1.24f, cz + 0.52f, 0.90f, 0.16f, 0.46f);

    // Tray stack
    dhCol(0.44f, 0.44f, 0.46f);
    for (int i = 0; i < 6; i++)
        DB(-13.5f, 0.60f + i * 0.05f, cz + 0.50f, 1.60f, 0.045f, 1.10f);

    // Menu board
    dhCol(0.18f, 0.18f, 0.20f); DB(-7.f, 3.40f, cz + 0.10f, 8.0f, 1.10f, 0.08f);
    dhCol(0.92f, 0.92f, 0.90f);
    for (int l = 0; l < 4; l++) DB(-7.f, 3.76f - l * 0.22f, cz + 0.14f, 6.8f, 0.05f, 0.04f);
    dhCol(0.80f, 0.22f, 0.18f); DB(-7.f, 3.84f, cz + 0.14f, 8.0f, 0.22f, 0.04f);

    // Cashier
    dhCol(0.30f, 0.28f, 0.26f); DB(13.f, 0.96f, cz + 0.56f, 1.80f, 1.92f, 0.90f);
    dhCol(0.88f, 0.88f, 0.86f); DB(13.f, 1.94f, cz + 0.52f, 1.84f, 0.06f, 0.94f);
    dhCol(0.14f, 0.14f, 0.16f); DB(13.f, 2.42f, cz + 0.20f, 0.70f, 0.48f, 0.06f);
    dhCol(0.20f, 0.60f, 0.90f); DB(13.f, 2.42f, cz + 0.18f, 0.58f, 0.36f, 0.04f);
}

// ==========================================================================
//  LEFT WALL: SINGLE CLEAN BASIN AREA
// ==========================================================================
static void dhBasinStation() {
    const float lx = DX1;
    const float bx = lx + 1.10f;
    const float bz = -6.0f;    const float rimY = 1.10f;
    const float bSc = 1.90f;

    // Counter base
    dhCol(0.52f, 0.49f, 0.46f);
    DB(bx, rimY * 0.5f, bz, 1.70f, rimY, 1.20f);
    // Counter top slab
    dhCol(0.68f, 0.66f, 0.63f);
    DB(bx, rimY + 0.035f, bz, 1.76f, 0.07f, 1.26f);

    // Bezier ceramic basin bowl
    dhDrawBasinMesh(bx, rimY + 0.08f, bz, bSc);

    // Faucet riser
    dhCol(0.82f, 0.80f, 0.78f);
    DB(bx, rimY + 0.50f, bz - 0.38f, 0.07f, 0.46f, 0.07f);
    // Spout arm
    DB(bx, rimY + 0.72f, bz - 0.18f, 0.06f, 0.06f, 0.42f);
    // Handle bar
    dhCol(0.62f, 0.60f, 0.64f);
    DB(bx, rimY + 0.78f, bz - 0.38f, 0.32f, 0.06f, 0.06f);
    // Hot knob (red)
    dhCol(0.85f, 0.26f, 0.22f);
    DB(bx - 0.15f, rimY + 0.78f, bz - 0.38f, 0.06f, 0.13f, 0.06f);
    // Cold knob (blue)
    dhCol(0.22f, 0.40f, 0.85f);
    DB(bx + 0.15f, rimY + 0.78f, bz - 0.38f, 0.06f, 0.13f, 0.06f);
    // Drain
    dhCol(0.38f, 0.36f, 0.36f);
    DB(bx, rimY + 0.09f, bz, 0.12f, 0.025f, 0.12f);

    // Mirror frame on wall
    dhCol(0.32f, 0.30f, 0.28f);
    DB(lx + 0.12f, rimY + 1.30f, bz, 0.08f, 1.10f, 0.92f);
    // Reflective face (light blue)
    dhCol(0.88f, 0.94f, 0.97f);
    DB(lx + 0.09f, rimY + 1.30f, bz, 0.05f, 0.94f, 0.78f);
    // Mirror light strip
    dhCol(1.00f, 0.97f, 0.88f);
    DB(lx + 0.11f, rimY + 1.92f, bz, 0.06f, 0.08f, 0.82f);

    // Soap dispenser
    dhCol(0.96f, 0.95f, 0.93f);
    DB(bx + 0.56f, rimY + 0.30f, bz - 0.28f, 0.14f, 0.34f, 0.14f);
    dhCol(0.28f, 0.64f, 0.44f);
    DB(bx + 0.56f, rimY + 0.50f, bz - 0.28f, 0.09f, 0.10f, 0.09f);

    // Towel rail
    dhCol(0.68f, 0.66f, 0.66f);
    DB(lx + 0.13f, rimY + 0.60f, bz + 0.58f, 0.07f, 0.06f, 0.62f);
    dhCol(0.96f, 0.96f, 0.94f);
    DB(lx + 0.14f, rimY + 0.56f, bz + 0.58f, 0.06f, 0.30f, 0.54f);

    // "WASH HANDS" sign
    dhCol(0.16f, 0.44f, 0.72f);
    DB(lx + 0.10f, rimY + 2.52f, bz, 0.06f, 0.22f, 1.20f);
    dhCol(0.97f, 0.97f, 0.95f);
    DB(lx + 0.09f, rimY + 2.52f, bz, 0.04f, 0.12f, 0.90f);

    // Overhead light bar
    dhCol(0.84f, 0.84f, 0.82f);
    DB(lx + 0.55f, DY - 0.06f, bz, 0.28f, 0.10f, 1.60f);
    dhCol(1.00f, 0.98f, 0.90f);
    DB(lx + 0.55f, DY - 0.08f, bz, 0.18f, 0.04f, 1.40f);
}

// ==========================================================================
//  DINING CHAIR
// ==========================================================================
static void drawDiningChair(float cx, float cz, float faceDeg) {
    float rad = glm::radians(faceDeg);
    float cr = cosf(rad), sr = sinf(rad);

    auto BL = [&](float lx, float ly, float lz,
        float sx, float sy, float sz) {
            float wx = cx + lx * cr - lz * sr;
            float wz = cz + lx * sr + lz * cr;
            glm::mat4 m = glm::translate(glm::mat4(1.f), { wx, ly, wz });
            m = glm::rotate(m, rad, { 0.f, 1.f, 0.f });
            m = glm::scale(m, { sx, sy, sz });
            dhSetModel(m); dhRenderCube();
        };

    dhCol(0.28f, 0.36f, 0.56f);
    BL(0, 1.02f, 0, 0.46f, 0.08f, 0.46f);
    dhCol(0.34f, 0.42f, 0.62f);
    BL(0, 1.07f, 0, 0.40f, 0.04f, 0.40f);
    dhCol(0.28f, 0.36f, 0.56f);
    BL(0, 1.72f, -0.21f, 0.44f, 0.70f, 0.06f);
    dhCol(0.38f, 0.38f, 0.40f);
    BL(-0.20f, 1.52f, -0.21f, 0.045f, 1.00f, 0.045f);
    BL(0.20f, 1.52f, -0.21f, 0.045f, 1.00f, 0.045f);
    BL(0, 1.00f, -0.21f, 0.48f, 0.06f, 0.045f);
    BL(0, 1.00f, 0.21f, 0.48f, 0.06f, 0.045f);
    BL(-0.21f, 1.00f, 0, 0.045f, 0.06f, 0.44f);
    BL(0.21f, 1.00f, 0, 0.045f, 0.06f, 0.44f);

    float lp[4][2] = { {-0.20f,-0.20f},{-0.20f,0.20f},{0.20f,-0.20f},{0.20f,0.20f} };
    for (auto& l : lp) {
        dhCol(0.38f, 0.38f, 0.40f);
        BL(l[0], 0.50f, l[1], 0.045f, 0.96f, 0.045f);
        dhCol(0.18f, 0.18f, 0.18f);
        BL(l[0], 0.02f, l[1], 0.055f, 0.05f, 0.055f);
    }
    dhCol(0.38f, 0.38f, 0.40f);
    BL(0, 0.42f, -0.20f, 0.40f, 0.040f, 0.040f);
    BL(0, 0.42f, 0.20f, 0.40f, 0.040f, 0.040f);
}

// ==========================================================================
//  CONDIMENTS
// ==========================================================================
static void drawCondiments(float tx, float tz) {
    const float TY = 1.06f;
    dhCol(0.94f, 0.94f, 0.94f); DB(tx - 0.22f, TY + 0.08f, tz, 0.07f, 0.18f, 0.07f);
    dhCol(0.64f, 0.64f, 0.64f); DB(tx - 0.22f, TY + 0.17f, tz, 0.08f, 0.04f, 0.08f);
    dhCol(0.32f, 0.32f, 0.32f); DB(tx + 0.22f, TY + 0.08f, tz, 0.07f, 0.18f, 0.07f);
    dhCol(0.64f, 0.64f, 0.64f); DB(tx + 0.22f, TY + 0.17f, tz, 0.08f, 0.04f, 0.08f);
    dhCol(0.84f, 0.16f, 0.14f); DB(tx + 0.40f, TY + 0.10f, tz + 0.08f, 0.08f, 0.22f, 0.08f);
    dhCol(0.50f, 0.68f, 0.56f); DB(tx - 0.40f, TY + 0.08f, tz - 0.08f, 0.07f, 0.18f, 0.07f);
}

// ==========================================================================
//  DINING TABLE  (4.0 wide x 2.0 deep, top at y=1.0)
//  6 chairs: 2 per long side + 1 per short end
// ==========================================================================
static void drawDiningTable(float tx, float tz) {
    const float TW = 6.2f;
    const float TD = 3.2f;
    const float TTY = 1.00f;
    const float TSLAB = 0.10f;

    // Tabletop
    dhCol(0.80f, 0.66f, 0.46f);
    DB(tx, TTY, tz, TW, TSLAB, TD);
    dhCol(0.88f, 0.76f, 0.56f);
    DB(tx, TTY + TSLAB * 0.5f, tz, TW * 0.96f, 0.012f, TD * 0.96f);

    // Edge trim
    dhCol(0.60f, 0.48f, 0.32f);
    DB(tx, TTY, tz + TD * 0.5f, TW, TSLAB, 0.018f);
    DB(tx, TTY, tz - TD * 0.5f, TW, TSLAB, 0.018f);
    DB(tx + TW * 0.5f, TTY, tz, 0.018f, TSLAB, TD);
    DB(tx - TW * 0.5f, TTY, tz, 0.018f, TSLAB, TD);

    // Apron
    dhCol(0.54f, 0.42f, 0.28f);
    DB(tx, TTY - 0.10f, tz + TD * 0.5f - 0.05f, TW - 0.20f, 0.12f, 0.04f);
    DB(tx, TTY - 0.10f, tz - TD * 0.5f + 0.05f, TW - 0.20f, 0.12f, 0.04f);
    DB(tx + TW * 0.5f - 0.05f, TTY - 0.10f, tz, 0.04f, 0.12f, TD - 0.20f);
    DB(tx - TW * 0.5f + 0.05f, TTY - 0.10f, tz, 0.04f, 0.12f, TD - 0.20f);

    // Legs
    dhCol(0.48f, 0.36f, 0.22f);
    float legOffX = TW * 0.5f - 0.18f;
    float legOffZ = TD * 0.5f - 0.18f;
    float legH = TTY - TSLAB * 0.5f;
    float lp[4][2] = {
        {-legOffX,-legOffZ},{-legOffX,+legOffZ},
        {+legOffX,-legOffZ},{+legOffX,+legOffZ}
    };
    for (auto& l : lp) {
        DB(tx + l[0], legH * 0.5f, tz + l[1], 0.08f, legH, 0.08f);
        dhCol(0.20f, 0.18f, 0.18f);
        DB(tx + l[0], 0.03f, tz + l[1], 0.10f, 0.06f, 0.10f);
        dhCol(0.48f, 0.36f, 0.22f);
    }

    drawCondiments(tx, tz);

    // 6 chairs
    float CR_N = TD * 0.5f + 0.54f;
    float CR_S = -TD * 0.5f - 0.54f;
    float CR_E = TW * 0.5f + 0.54f;
    float CR_W = -TW * 0.5f - 0.54f;

    drawDiningChair(tx - 1.00f, tz + CR_N, 180.f);
    drawDiningChair(tx + 1.00f, tz + CR_N, 180.f);
    drawDiningChair(tx - 1.00f, tz + CR_S, 0.f);
    drawDiningChair(tx + 1.00f, tz + CR_S, 0.f);
    drawDiningChair(tx + CR_E, tz + 0.3f, 90.f);
    drawDiningChair(tx + CR_W, tz + 0.3f, 270.f);
}

// One table centred in room
static void dhTableGrid() {
    drawDiningTable(0.f, 0.f);
}

// ==========================================================================
//  CEILING LIGHTS
// ==========================================================================
static void dhCeilingLights() {
    float bankX[] = { -6.f, 6.f };
    for (float bx : bankX) {
        dhCol(0.80f, 0.80f, 0.80f); DB(bx, DY - 0.04f, 0.f, 0.34f, 0.08f, 18.f);
        dhCol(1.0f, 0.98f, 0.92f); DB(bx, DY - 0.06f, 0.f, 0.24f, 0.04f, 17.6f);
    }
    for (int p = 0; p < 5; p++) {
        float px = -10.f + p * 5.f;
        dhCol(0.44f, 0.44f, 0.46f); DB(px, DY - 0.60f, DZ1 + 0.56f, 0.025f, 1.10f, 0.025f);
        dhCol(0.92f, 0.90f, 0.88f); DB(px, DY - 1.16f, DZ1 + 0.56f, 0.50f, 0.18f, 0.50f);
        dhCol(1.0f, 0.97f, 0.88f); DB(px, DY - 1.22f, DZ1 + 0.56f, 0.18f, 0.08f, 0.18f);
    }
}

// ==========================================================================
//  CEILING FANS (New Design)
// ==========================================================================
static void drawDhFan(float fx, float fz) {
    // Lowered fan so ceiling panel no longer intersects motor/rod
    float fY2 = DY - 1.25f;
    float t = (float)glfwGetTime();
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { fx, fY2 + 0.96f, fz }), { 0.28f, 0.18f, 0.28f });
    drawCylinder(dh_sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
    for (int b = 0; b < 4; b++) {
        float ang = t * 100.f + b * 90.f;
        glm::mat4 blade = glm::translate(glm::mat4(1.f), { fx, fY2 + 0.95f, fz });
        blade = glm::rotate(blade, glm::radians(ang), { 0.f, 1.f, 0.f });
        blade = glm::translate(blade, { 1.0f, 0.f, 0.f });
        blade = glm::scale(blade, { 1.50f, 0.05f, 0.38f });
        drawCube(dh_sh, blade, glm::vec3(0.48f, 0.38f, 0.24f), texWood, 1.f);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { fx, fY2 + 1.06f, fz }), { 0.05f, 0.18f, 0.05f });
    drawCylinder(dh_sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
}

static void dhCeilingFans() {
    drawDhFan(-5.f, 0.f);
    drawDhFan(5.f, 0.f);
}

// ==========================================================================
//  WALL CLOCK
// ==========================================================================
static void dhWallClock() {
    float t = (float)glfwGetTime();
    float cx = 11.5f, cy = 3.60f, cz = DZ1 + 0.16f;
    dhCol(0.20f, 0.20f, 0.22f); DB(cx, cy, cz, 0.08f, 0.76f, 0.76f);
    dhCol(0.97f, 0.96f, 0.94f); DB(cx, cy, cz + 0.05f, 0.06f, 0.64f, 0.64f);
    dhCol(0.28f, 0.26f, 0.24f);
    for (int h = 0; h < 12; h++) {
        float a = h * 30.f * 3.14159f / 180.f;
        DB(cx + 0.05f, cy + cosf(a) * 0.24f, cz + sinf(a) * 0.24f + 0.06f,
            0.04f, 0.034f, 0.028f);
    }
    dhCol(0.16f, 0.14f, 0.12f);
    {
        float ang = -t * 3.f * 3.14159f / 180.f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { cx + 0.06f, cy, cz + 0.07f });
        m = glm::rotate(m, ang, { 1.f, 0.f, 0.f });
        m = glm::translate(m, { 0.f, 0.088f, 0.f });
        m = glm::scale(m, { 0.034f, 0.18f, 0.034f });
        dhSetModel(m); dhRenderCube();
    }
    {
        float ang = -t * 36.f * 3.14159f / 180.f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { cx + 0.06f, cy, cz + 0.07f });
        m = glm::rotate(m, ang, { 1.f, 0.f, 0.f });
        m = glm::translate(m, { 0.f, 0.125f, 0.f });
        m = glm::scale(m, { 0.024f, 0.25f, 0.024f });
        dhSetModel(m); dhRenderCube();
    }
    dhCol(0.70f, 0.14f, 0.12f);
    DB(cx + 0.07f, cy, cz + 0.08f, 0.042f, 0.042f, 0.042f);
}

// ==========================================================================
//  CORNER PLANTS
// ==========================================================================
static void dhPlants() {
    auto plant = [&](float px, float pz) {
        dhCol(0.62f, 0.28f, 0.12f); DB(px, 0.58f, pz, 0.44f, 0.76f, 0.44f);
        dhCol(0.22f, 0.16f, 0.10f); DB(px, 1.00f, pz, 0.40f, 0.055f, 0.40f);
        for (int i = 0; i < 5; i++) {
            float a = i * (360.f / 5) * 3.14159f / 180.f;
            float ox = cosf(a) * 0.11f, oz = sinf(a) * 0.11f;
            dhCol(0.22f, 0.54f, 0.24f);
            DB(px + ox, 1.35f, pz + oz, 0.070f, 0.58f, 0.070f);
            dhCol(0.26f, 0.60f, 0.28f);
            DB(px + ox * 1.5f, 1.72f, pz + oz * 1.5f, 0.22f, 0.15f, 0.22f);
        }
        };
    plant(DX2 - 1.2f, DZ2 - 1.2f);
    plant(DX2 - 1.2f, DZ1 + 1.8f);
}

// ==========================================================================
//  PUBLIC API
// ==========================================================================

void drawDiningHall() {
    // Set camera once per room entry — middle of room, eye-level, looking toward back wall
    if (!dhCameraSet) {
        dhSetTopViewCamera();
        dhCameraSet = true;
    }

    GLint cur = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
    dh_sh = (unsigned int)cur;

    dhLighting();
    dhShell();
    dhRightWindows();
    dhEntrance();
    dhServingCounter();
    dhBasinStation();
    dhTableGrid();
    dhCeilingLights();
    dhCeilingFans();
    dhWallClock();
    dhPlants();
}

void updateDiningHallFan(float dt) {
    dhFanAngle += DH_FAN_DEG_PER_SEC * dt;
    if (dhFanAngle >= 360.f) dhFanAngle -= 360.f;
}

void updateDiningHallFan() {
    updateDiningHallFan(deltaTime);
}

void toggleDiningHallLight() {
    dhLightOn = !dhLightOn;
}

// Call when leaving dining room so camera resets on next entry
void resetDiningHallCamera() {
    dhCameraSet = false;
}