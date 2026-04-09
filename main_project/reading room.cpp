// reading_room.cpp  —  Modern University Library Reading Room
// Reference: bright open library with large windows LEFT wall,
//            low bookshelves RIGHT + BACK walls (matching photo),
//            square white tables (2×2 grid = 4 sets, well-spaced),
//            light-wood chairs with mint-green seat pads,
//            white polished floor, warm ceiling lights.
//
// CAMERA: Set your camera to top-down view for best result:
//   camera.Position = glm::vec3(0.f, 20.f, 0.f);
//   camera.Pitch = -89.f;  camera.Yaw = -90.f;
//
// Public API:  drawReadingRoom()  updateReadingRoomFan()  toggleReadingRoomLight()
// ─────────────────────────────────────────────────────────────────────────────

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "globals.h"
#include "room.h"
#include "draw_helpers.h"

#include <cmath>
#include <cstdio>

// ══════════════════════════════════════════════════════════════════════════
//  MODULE GLOBALS
// ══════════════════════════════════════════════════════════════════════════
float rrFanAngle = 0.0f;
bool  rrLightOn = true;
static unsigned int rr_sh = 0;

// ══════════════════════════════════════════════════════════════════════════
//  ROOM LAYOUT CONSTANTS
//  X : -12 … +12  (width  24 units)
//  Z : -12 … +12  (depth  24 units)
//  Y :   0 …  5   (ceiling at 5)
//
//  LEFT  wall  (X = -12) : floor-to-ceiling glass windows
//  RIGHT wall  (X = +12) : low bookshelves (photo accurate)
//  BACK  wall  (Z = -12) : tall bookshelves
//  FRONT wall  (Z = +12) : entry wall
// ══════════════════════════════════════════════════════════════════════════
static const float RX1 = -12.f, RX2 = 12.f;
static const float RZ1 = -12.f, RZ2 = 12.f;
static const float RY = 5.f;

// ── Helpers ──────────────────────────────────────────────────────────────

static void renderCube() {
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void col(float r, float g, float b) {
    glUniform3f(glGetUniformLocation(rr_sh, "objectColor"), r, g, b);
    glUniform1i(glGetUniformLocation(rr_sh, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(rr_sh, "textureMode"), 0);
}

static void setModel(glm::mat4 m) {
    glUniformMatrix4fv(glGetUniformLocation(rr_sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
    glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(m)));
    int nl = glGetUniformLocation(rr_sh, "normalMatrix");
    if (nl >= 0) glUniformMatrix3fv(nl, 1, GL_FALSE, glm::value_ptr(nm));
}

// Box centred at (cx,cy,cz), full dimensions sx×sy×sz
static void B(float cx, float cy, float cz,
    float sx, float sy, float sz) {
    glm::mat4 m = glm::translate(glm::mat4(1.f), { cx, cy, cz });
    m = glm::scale(m, { sx, sy, sz });
    setModel(m);
    renderCube();
}

// Box with Y-axis rotation (degrees)
static void BR(float cx, float cy, float cz,
    float sx, float sy, float sz, float deg) {
    glm::mat4 m = glm::translate(glm::mat4(1.f), { cx,cy,cz });
    m = glm::rotate(m, glm::radians(deg), { 0.f,1.f,0.f });
    m = glm::scale(m, { sx,sy,sz });
    setModel(m);
    renderCube();
}

// ══════════════════════════════════════════════════════════════════════════
//  LIGHTING
// ══════════════════════════════════════════════════════════════════════════
static void rrLighting() {
    glm::vec3 amb = rrLightOn ? glm::vec3(0.55f, 0.54f, 0.52f)
        : glm::vec3(0.06f, 0.06f, 0.06f);
    glUniform3fv(glGetUniformLocation(rr_sh, "ambientColor"), 1, glm::value_ptr(amb));

    glUniform3f(glGetUniformLocation(rr_sh, "lightPos"), 0.f, 7.f, 0.f);
    glUniform3f(glGetUniformLocation(rr_sh, "lightColor"), 1.0f, 0.98f, 0.95f);
    glUniform1f(glGetUniformLocation(rr_sh, "lightIntensity"), rrLightOn ? 1.6f : 0.f);

    // Daylight from left windows
    glUniform3f(glGetUniformLocation(rr_sh, "lightPos2"), RX1 + 0.5f, 3.5f, 0.f);
    glUniform3f(glGetUniformLocation(rr_sh, "lightColor2"), 1.0f, 0.97f, 0.90f);
    glUniform1f(glGetUniformLocation(rr_sh, "lightIntensity2"), rrLightOn ? 0.85f : 0.f);
}

// ══════════════════════════════════════════════════════════════════════════
//  ROOM SHELL
// ══════════════════════════════════════════════════════════════════════════
static void rrShell() {
    float W = RX2 - RX1;   // 24
    float D = RZ2 - RZ1;   // 24
    float mx = 0.f, mz = 0.f;

    // ── Floor: polished off-white large tiles (3×3 unit) ─────────────────
    int tilesX = 8, tilesZ = 8;
    for (int i = 0; i < tilesX; i++) for (int j = 0; j < tilesZ; j++) {
        float v = ((i + j) % 2 == 0) ? 0.97f : 0.94f;
        col(v, v, v);
        float tx = RX1 + i * 3.f + 1.5f;
        float tz = RZ1 + j * 3.f + 1.5f;
        B(tx, 0.01f, tz, 2.98f, 0.04f, 2.98f);
    }
    // Grout lines
    col(0.80f, 0.80f, 0.80f);
    B(mx, 0.0f, mz, W, 0.02f, D);

    // ── Ceiling (bright white) ────────────────────────────────────────────
    col(0.97f, 0.97f, 0.97f);
    B(mx, RY, mz, W, 0.08f, D);

    // ── Left wall (backing for windows) ──────────────────────────────────
    col(0.94f, 0.96f, 0.99f);
    B(RX1, RY * 0.5f, mz, 0.10f, RY, D);

    // ── Right wall ────────────────────────────────────────────────────────
    col(0.92f, 0.91f, 0.90f);
    B(RX2, RY * 0.5f, mz, 0.10f, RY, D);

    // ── Back wall ─────────────────────────────────────────────────────────
    col(0.91f, 0.90f, 0.88f);
    B(mx, RY * 0.5f, RZ1, W, RY, 0.10f);

    // ── Front wall ────────────────────────────────────────────────────────
    col(0.94f, 0.93f, 0.91f);
    B(mx, RY * 0.5f, RZ2, W, RY, 0.10f);

    // ── Ceiling recessed grid (structural look) ───────────────────────────
    col(0.87f, 0.87f, 0.87f);
    for (int i = 0; i <= 8; i++) {
        float x = RX1 + i * 3.f;
        B(x, RY - 0.01f, mz, 0.05f, 0.06f, D);
    }
    for (int j = 0; j <= 8; j++) {
        float z = RZ1 + j * 3.f;
        B(mx, RY - 0.01f, z, W, 0.06f, 0.05f);
    }

    // ── Floor skirting ────────────────────────────────────────────────────
    col(0.76f, 0.72f, 0.66f);
    B(mx, 0.18f, RZ1 + 0.05f, W, 0.30f, 0.08f);
    B(mx, 0.18f, RZ2 - 0.05f, W, 0.30f, 0.08f);
    B(RX2 - 0.05f, 0.18f, mz, 0.08f, 0.30f, D);
    B(RX1 + 0.05f, 0.18f, mz, 0.08f, 0.30f, D);
}

// ══════════════════════════════════════════════════════════════════════════
//  LEFT WALL: FLOOR-TO-CEILING GLASS WINDOWS
// ══════════════════════════════════════════════════════════════════════════
static void rrLeftWindows() {
    float lx = RX1;

    col(0.90f, 0.90f, 0.90f);
    for (float z = RZ1; z <= RZ2 + 0.1f; z += 4.f)
        B(lx + 0.20f, RY * 0.5f, z, 0.30f, RY + 0.1f, 0.26f);

    for (int p = 0; p < 6; p++) {
        float z1 = RZ1 + p * 4.f;
        float zc = z1 + 2.f;

        col(0.68f, 0.83f, 0.95f);
        B(lx - 0.02f, RY * 0.5f + 0.3f, zc, 0.06f, RY - 0.6f, 3.70f);

        col(0.85f, 0.93f, 0.99f);
        B(lx + 0.12f, RY * 0.5f, zc, 0.04f, RY - 0.18f, 3.68f);

        col(0.86f, 0.86f, 0.85f);
        B(lx + 0.14f, 2.5f, zc, 0.06f, 0.10f, 3.68f);

        col(0.82f, 0.80f, 0.76f);
        B(lx + 0.32f, 0.34f, zc, 0.50f, 0.07f, 3.68f);
    }

    col(0.74f, 0.72f, 0.70f);
    B(lx + 0.44f, 0.04f, 0.f, 0.70f, 0.06f, RZ2 - RZ1);
}

// ══════════════════════════════════════════════════════════════════════════
//  RIGHT WALL: LOW BOOKSHELVES
// ══════════════════════════════════════════════════════════════════════════
static void rrRightShelves() {
    float rx = RX2;

    static const float BOOK_COLS[12][3] = {
        {0.72f,0.18f,0.12f},{0.18f,0.36f,0.68f},{0.24f,0.58f,0.26f},
        {0.72f,0.62f,0.14f},{0.50f,0.18f,0.58f},{0.60f,0.32f,0.18f},
        {0.18f,0.54f,0.54f},{0.66f,0.22f,0.36f},{0.28f,0.44f,0.22f},
        {0.54f,0.40f,0.12f},{0.40f,0.22f,0.60f},{0.62f,0.50f,0.20f}
    };

    for (int u = 0; u < 6; u++) {
        float z1 = RZ1 + u * 4.f;
        float zc = z1 + 2.f;

        col(0.65f, 0.52f, 0.34f);
        B(rx - 0.55f, 0.06f, zc, 0.90f, 0.10f, 3.90f);
        col(0.70f, 0.56f, 0.37f);
        B(rx - 0.55f, 1.85f, zc, 0.94f, 0.10f, 3.94f);
        col(0.60f, 0.48f, 0.30f);
        B(rx - 0.55f, 0.96f, z1 + 0.05f, 0.90f, 1.80f, 0.09f);
        B(rx - 0.55f, 0.96f, z1 + 3.95f, 0.90f, 1.80f, 0.09f);
        col(0.55f, 0.44f, 0.27f);
        B(rx - 0.22f, 0.96f, zc, 0.08f, 1.80f, 3.80f);

        col(0.65f, 0.52f, 0.34f);
        B(rx - 0.55f, 0.96f, zc, 0.88f, 0.08f, 3.80f);

        for (int b = 0; b < 11; b++) {
            float bz = z1 + 0.25f + b * 0.32f;
            float bh = 0.50f + float((b + u) % 4) * 0.08f;
            float bw = 0.06f + float((b * 2 + u) % 3) * 0.015f;
            const float* c = BOOK_COLS[(b + u * 3) % 12];
            col(c[0], c[1], c[2]);
            B(rx - 0.56f, 0.11f + bh * 0.5f, bz, bw, bh, 0.26f);
            col(c[0] + 0.14f, c[1] + 0.14f, c[2] + 0.14f);
            B(rx - 0.55f, 0.11f + bh * 0.5f, bz, 0.02f, bh * 0.28f, 0.20f);
        }

        for (int b = 0; b < 11; b++) {
            float bz = z1 + 0.25f + b * 0.32f;
            float bh = 0.48f + float((b + u + 1) % 4) * 0.08f;
            float bw = 0.06f + float((b * 3 + u) % 3) * 0.015f;
            const float* c = BOOK_COLS[(b + u * 2 + 5) % 12];
            col(c[0], c[1], c[2]);
            B(rx - 0.56f, 1.00f + bh * 0.5f, bz, bw, bh, 0.26f);
            col(c[0] + 0.14f, c[1] + 0.14f, c[2] + 0.14f);
            B(rx - 0.55f, 1.00f + bh * 0.5f, bz, 0.02f, bh * 0.28f, 0.20f);
        }
    }

    col(0.94f, 0.93f, 0.91f);
    B(rx - 0.12f, 3.4f, 0.f, 0.18f, 3.2f, RZ2 - RZ1);
}

// ══════════════════════════════════════════════════════════════════════════
//  BACK WALL: TALL BOOKSHELVES
// ══════════════════════════════════════════════════════════════════════════
static void rrBackShelves() {
    float bz = RZ1;

    static const float BC[12][3] = {
        {0.72f,0.18f,0.12f},{0.18f,0.36f,0.68f},{0.24f,0.58f,0.26f},
        {0.72f,0.62f,0.14f},{0.50f,0.18f,0.58f},{0.60f,0.32f,0.18f},
        {0.18f,0.54f,0.54f},{0.66f,0.22f,0.36f},{0.28f,0.44f,0.22f},
        {0.54f,0.40f,0.12f},{0.40f,0.22f,0.60f},{0.62f,0.50f,0.20f}
    };

    for (int u = 0; u < 5; u++) {
        float xc = RX1 + 2.4f + u * 4.8f;
        float hw = 4.2f;
        float depth = 0.90f;

        col(0.55f, 0.44f, 0.26f);
        B(xc - hw * 0.5f - 0.05f, 2.5f, bz + depth * 0.5f, 0.10f, 5.0f, depth);
        B(xc + hw * 0.5f + 0.05f, 2.5f, bz + depth * 0.5f, 0.10f, 5.0f, depth);
        col(0.50f, 0.40f, 0.22f);
        B(xc, 2.5f, bz + 0.10f, hw, 5.0f, 0.08f);
        col(0.66f, 0.54f, 0.34f);
        B(xc, 5.07f, bz + depth * 0.5f, hw + 0.24f, 0.12f, depth + 0.04f);
        col(0.62f, 0.50f, 0.31f);
        B(xc, 0.05f, bz + depth * 0.5f, hw + 0.10f, 0.10f, depth);

        col(0.62f, 0.50f, 0.31f);
        for (int s = 0; s < 4; s++)
            B(xc, 0.44f + s * 1.14f, bz + depth * 0.5f, hw, 0.08f, depth);

        for (int s = 0; s < 4; s++) {
            float sy = 0.50f + s * 1.14f;
            int booksPerShelf = 11;
            for (int b = 0; b < booksPerShelf; b++) {
                float bx = xc - hw * 0.5f + 0.2f + b * 0.36f;
                float bh = 0.52f + float((b + s + u) % 4) * 0.10f;
                float bw = 0.06f + float((b * 2 + s) % 3) * 0.02f;
                const float* c = BC[(b + s * 3 + u * 2) % 12];
                col(c[0], c[1], c[2]);
                B(bx, sy + bh * 0.5f, bz + 0.60f, bw, bh, 0.30f);
                col(c[0] + 0.12f, c[1] + 0.12f, c[2] + 0.12f);
                B(bx, sy + bh * 0.5f, bz + 0.64f, 0.02f, bh * 0.26f, 0.24f);
            }
        }

        col(0.55f, 0.44f, 0.26f);
        B(xc, 2.5f, bz + 0.50f, 0.06f, 5.0f, 0.06f);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  CHAIR
// ══════════════════════════════════════════════════════════════════════════
static void drawOneChair(float cx, float cz, float faceDeg) {
    float rad = glm::radians(faceDeg);
    float cr = cosf(rad), sr = sinf(rad);

    auto BL = [&](float lx, float ly, float lz,
        float sx, float sy, float sz) {
            float wx = cx + lx * cr - lz * sr;
            float wz = cz + lx * sr + lz * cr;
            glm::mat4 m = glm::translate(glm::mat4(1.f), { wx,ly,wz });
            m = glm::rotate(m, rad, { 0.f,1.f,0.f });
            m = glm::scale(m, { sx,sy,sz });
            setModel(m);
            renderCube();
        };

    col(0.70f, 0.82f, 0.74f);
    BL(0, 1.04f, 0, 0.48f, 0.09f, 0.48f);
    col(0.76f, 0.88f, 0.80f);
    BL(0, 1.10f, 0, 0.42f, 0.04f, 0.42f);

    col(0.80f, 0.70f, 0.48f);
    BL(-0.19f, 1.54f, -0.22f, 0.055f, 1.00f, 0.055f);
    BL(0.19f, 1.54f, -0.22f, 0.055f, 1.00f, 0.055f);

    col(0.78f, 0.68f, 0.46f);
    BL(0, 1.98f, -0.22f, 0.46f, 0.08f, 0.08f);
    BL(0, 1.58f, -0.22f, 0.40f, 0.06f, 0.06f);
    BL(0, 1.26f, -0.22f, 0.40f, 0.05f, 0.05f);

    col(0.80f, 0.70f, 0.48f);
    BL(0, 1.00f, -0.21f, 0.50f, 0.07f, 0.055f);
    BL(0, 1.00f, 0.21f, 0.50f, 0.07f, 0.055f);
    BL(-0.21f, 1.00f, 0, 0.055f, 0.07f, 0.42f);
    BL(0.21f, 1.00f, 0, 0.055f, 0.07f, 0.42f);

    col(0.78f, 0.68f, 0.46f);
    float lp[4][2] = { {-0.19f,-0.19f},{-0.19f,0.19f},{0.19f,-0.19f},{0.19f,0.19f} };
    for (auto& l : lp) {
        BL(l[0], 0.50f, l[1], 0.055f, 0.96f, 0.055f);
        col(0.68f, 0.58f, 0.38f);
        BL(l[0], 0.03f, l[1], 0.065f, 0.07f, 0.065f);
        col(0.78f, 0.68f, 0.46f);
    }

    col(0.74f, 0.64f, 0.44f);
    BL(0, 0.44f, -0.19f, 0.42f, 0.045f, 0.045f);
    BL(0, 0.44f, 0.19f, 0.42f, 0.045f, 0.045f);
    BL(-0.19f, 0.44f, 0, 0.045f, 0.045f, 0.42f);
    BL(0.19f, 0.44f, 0, 0.045f, 0.045f, 0.42f);
}

// ══════════════════════════════════════════════════════════════════════════
//  BOOKS ON TABLE  — flat open book, stacked closed books, upright book,
//                    pencil, water bottle (one set per table)
//  All coordinates are LOCAL to table centre (tx, tz).
// ══════════════════════════════════════════════════════════════════════════
static void drawTableBooks(float tx, float tz) {
    // ── Palette of book cover colours ────────────────────────────────────
    static const float BC[8][3] = {
        {0.72f,0.18f,0.12f},{0.18f,0.36f,0.68f},{0.24f,0.58f,0.26f},
        {0.72f,0.62f,0.14f},{0.50f,0.18f,0.58f},{0.18f,0.54f,0.54f},
        {0.66f,0.22f,0.36f},{0.54f,0.40f,0.12f}
    };

    // ── SEAT 1 (north side, tz+ quadrant) ────────────────────────────────
    {
        float bx = tx - 0.22f, bz = tz + 0.30f;

        // Open textbook (lying flat, spine crease visible)
        col(0.95f, 0.94f, 0.92f);           // pages (left leaf)
        B(bx - 0.14f, 1.575f, bz, 0.27f, 0.022f, 0.36f);
        col(0.93f, 0.92f, 0.90f);           // pages (right leaf, slight shadow)
        B(bx + 0.14f, 1.575f, bz, 0.27f, 0.022f, 0.36f);
        col(0.18f, 0.36f, 0.68f);           // cover colour bleeds into spine
        B(bx, 1.570f, bz, 0.025f, 0.028f, 0.36f);   // spine crease
        // Text lines on left page
        col(0.60f, 0.60f, 0.60f);
        for (int l = 0; l < 5; l++)
            B(bx - 0.14f, 1.588f, bz - 0.12f + l * 0.055f, 0.20f, 0.006f, 0.012f);
        // Text lines on right page
        for (int l = 0; l < 5; l++)
            B(bx + 0.14f, 1.588f, bz - 0.12f + l * 0.055f, 0.20f, 0.006f, 0.012f);

        // Pencil lying beside book
        col(0.94f, 0.82f, 0.22f);           // yellow pencil body
        BR(bx + 0.42f, 1.578f, bz + 0.04f, 0.08f, 0.014f, 0.014f, 15.f);
        col(0.90f, 0.70f, 0.56f);           // eraser end
        BR(bx + 0.48f, 1.578f, bz + 0.06f, 0.025f, 0.015f, 0.015f, 15.f);
        col(0.22f, 0.20f, 0.18f);           // graphite tip
        BR(bx + 0.36f, 1.578f, bz + 0.02f, 0.018f, 0.012f, 0.012f, 15.f);

        // Closed notebook (smaller, on top-right corner)
        col(BC[4][0], BC[4][1], BC[4][2]);
        B(bx + 0.38f, 1.580f, bz - 0.28f, 0.22f, 0.018f, 0.28f);
        col(BC[4][0] * 0.7f, BC[4][1] * 0.7f, BC[4][2] * 0.7f);
        B(bx + 0.38f, 1.580f, bz - 0.28f, 0.005f, 0.020f, 0.28f);  // spine
    }

    // ── SEAT 2 (south side, tz- quadrant) ────────────────────────────────
    {
        float bx = tx + 0.10f, bz = tz - 0.32f;

        // Stack of 3 closed books (bottom to top)
        float stackY = 1.560f;
        int order[3] = { 2, 0, 6 };
        float bw[3] = { 0.32f, 0.30f, 0.28f };
        float bd[3] = { 0.22f, 0.20f, 0.19f };
        for (int i = 0; i < 3; i++) {
            const float* c = BC[order[i]];
            col(c[0], c[1], c[2]);
            B(bx, stackY, bz, bw[i], 0.022f, bd[i]);
            // spine edge (darker strip on left)
            col(c[0] * 0.70f, c[1] * 0.70f, c[2] * 0.70f);
            B(bx - bw[i] * 0.5f + 0.006f, stackY, bz, 0.012f, 0.024f, bd[i]);
            stackY += 0.024f;
        }

        // Water bottle (clear cylinder approximated with tall thin box)
        col(0.80f, 0.92f, 0.98f);
        B(bx - 0.42f, 1.660f, bz + 0.10f, 0.095f, 0.20f, 0.095f);
        col(0.60f, 0.82f, 0.92f);           // slightly darker body
        B(bx - 0.42f, 1.595f, bz + 0.10f, 0.090f, 0.080f, 0.090f);
        col(0.34f, 0.62f, 0.80f);           // cap
        B(bx - 0.42f, 1.760f, bz + 0.10f, 0.060f, 0.040f, 0.060f);
    }

    // ── SEAT 3 (east side, tx+ quadrant) ─────────────────────────────────
    {
        float bx = tx + 0.30f, bz = tz - 0.10f;

        // Single open book (rotated ~20° for natural look)
        col(BC[1][0], BC[1][1], BC[1][2]);  // cover showing under pages
        BR(bx, 1.570f, bz, 0.56f, 0.022f, 0.38f, 20.f);
        col(0.96f, 0.95f, 0.93f);
        BR(bx - 0.10f, 1.576f, bz, 0.25f, 0.018f, 0.36f, 20.f);
        BR(bx + 0.10f, 1.576f, bz, 0.25f, 0.018f, 0.36f, 20.f);
        col(0.24f, 0.22f, 0.20f);
        BR(bx, 1.576f, bz, 0.022f, 0.020f, 0.36f, 20.f);  // spine
        // Highlighter lying on page
        col(0.86f, 0.96f, 0.28f);           // yellow highlighter
        BR(bx + 0.06f, 1.586f, bz - 0.06f, 0.14f, 0.013f, 0.022f, 38.f);
    }

    // ── SEAT 4 (west side, tx- quadrant) ─────────────────────────────────
    {
        float bx = tx - 0.32f, bz = tz + 0.12f;

        // Closed hardcover book (upright, leaning against nothing, flat down)
        col(BC[7][0], BC[7][1], BC[7][2]);
        B(bx, 1.572f, bz, 0.30f, 0.020f, 0.22f);
        col(BC[7][0] * 0.72f, BC[7][1] * 0.72f, BC[7][2] * 0.72f);
        B(bx - 0.148f, 1.572f, bz, 0.010f, 0.022f, 0.22f);

        // Sticky note (small square on top of book)
        col(0.98f, 0.94f, 0.42f);           // yellow sticky note
        B(bx + 0.04f, 1.585f, bz, 0.10f, 0.008f, 0.10f);
        col(0.50f, 0.50f, 0.50f);
        B(bx + 0.04f, 1.591f, bz - 0.02f, 0.07f, 0.005f, 0.008f);  // written line

        // Pen cap-down beside sticky note
        col(0.18f, 0.22f, 0.70f);           // blue pen
        B(bx + 0.22f, 1.600f, bz + 0.06f, 0.016f, 0.040f, 0.016f);
        col(0.10f, 0.10f, 0.10f);           // tip
        B(bx + 0.22f, 1.578f, bz + 0.06f, 0.010f, 0.008f, 0.010f);
    }

    // ── Centre of table: small decorative object (plant pot / eraser) ─────
    // Tiny succulent pot at exact centre — unobtrusive
    col(0.70f, 0.56f, 0.40f);              // terracotta pot
    B(tx, 1.600f, tz, 0.10f, 0.090f, 0.10f);
    col(0.24f, 0.54f, 0.28f);              // plant body
    B(tx, 1.660f, tz, 0.075f, 0.080f, 0.075f);
    // tiny leaves
    col(0.28f, 0.60f, 0.30f);
    B(tx - 0.04f, 1.710f, tz, 0.060f, 0.030f, 0.025f);
    B(tx + 0.04f, 1.710f, tz, 0.060f, 0.030f, 0.025f);
    B(tx, 1.710f, tz - 0.04f, 0.025f, 0.030f, 0.060f);
}

// ══════════════════════════════════════════════════════════════════════════
//  TABLE SET (1 square table + 4 chairs + books)
// ══════════════════════════════════════════════════════════════════════════
static void drawTableSet(float tx, float tz) {
    // ── Tabletop (pure white, 1.70×1.70) ─────────────────────────────────
    col(0.96f, 0.96f, 0.96f);
    B(tx, 1.50f, tz, 1.70f, 0.08f, 1.70f);
    col(0.99f, 0.99f, 0.99f);
    B(tx, 1.55f, tz, 1.70f, 0.02f, 1.70f);

    // Apron
    col(0.86f, 0.78f, 0.58f);
    B(tx, 1.43f, tz + 0.83f, 1.52f, 0.09f, 0.05f);
    B(tx, 1.43f, tz - 0.83f, 1.52f, 0.09f, 0.05f);
    B(tx + 0.83f, 1.43f, tz, 0.05f, 0.09f, 1.52f);
    B(tx - 0.83f, 1.43f, tz, 0.05f, 0.09f, 1.52f);

    // ── Four slim legs ────────────────────────────────────────────────────
    col(0.80f, 0.70f, 0.48f);
    float leg[4][2] = { {-0.78f,-0.78f},{-0.78f,0.78f},{0.78f,-0.78f},{0.78f,0.78f} };
    for (auto& l : leg) {
        B(tx + l[0], 0.74f, tz + l[1], 0.065f, 1.40f, 0.065f);
        col(0.68f, 0.58f, 0.38f);
        B(tx + l[0], 0.03f, tz + l[1], 0.075f, 0.07f, 0.075f);
        col(0.80f, 0.70f, 0.48f);
    }

    // ── Books, stationery, bottles ────────────────────────────────────────
    drawTableBooks(tx, tz);

    // ── 4 chairs — one per side ───────────────────────────────────────────
    const float R = 1.38f;
    drawOneChair(tx, tz + R, 180.f);
    drawOneChair(tx, tz - R, 0.f);
    drawOneChair(tx + R, tz, 90.f);
    drawOneChair(tx - R, tz, 270.f);
}

// ══════════════════════════════════════════════════════════════════════════
//  TABLE GRID  — 2×2 = 4 table sets
// ══════════════════════════════════════════════════════════════════════════
static void rrTableGrid() {
    const float TX[] = { -2.5f, 4.5f };
    const float TZ[] = { -5.0f, 3.0f };

    for (float tx : TX)
        for (float tz : TZ)
            drawTableSet(tx, tz);
}

// ══════════════════════════════════════════════════════════════════════════
//  CEILING RECESSED LED LIGHTS
// ══════════════════════════════════════════════════════════════════════════
static void rrCeilingLights() {
    const float TX[] = { -2.5f, 4.5f };
    const float TZ[] = { -5.0f, 3.0f };
    for (float x : TX) for (float z : TZ) {
        col(0.85f, 0.85f, 0.84f);
        B(x, RY - 0.02f, z, 1.10f, 0.06f, 0.75f);
        col(1.0f, 0.98f, 0.92f);
        B(x, RY - 0.03f, z, 1.00f, 0.03f, 0.65f);
    }
    col(0.90f, 0.90f, 0.90f);
    B(0.f, RY - 0.04f, RZ1 + 1.0f, 22.f, 0.04f, 0.16f);
    B(0.f, RY - 0.04f, RZ2 - 1.0f, 22.f, 0.04f, 0.16f);
    B(RX1 + 1.f, RY - 0.04f, 0.f, 0.16f, 0.04f, 22.f);
    B(RX2 - 1.f, RY - 0.04f, 0.f, 0.16f, 0.04f, 22.f);
}

// ══════════════════════════════════════════════════════════════════════════
//  CEILING FAN (New Design)
// ══════════════════════════════════════════════════════════════════════════
static void drawOneFan(float fx, float fz) {
    float fY2 = RY - 1.0f;
    float t = (float)glfwGetTime();
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { fx, fY2 + 0.96f, fz }), { 0.28f, 0.18f, 0.28f });
    drawCylinder(rr_sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
    for (int b = 0; b < 4; b++) {
        float ang = t * 100.f + b * 90.f;
        glm::mat4 blade = glm::translate(glm::mat4(1.f), { fx, fY2 + 0.95f, fz });
        blade = glm::rotate(blade, glm::radians(ang), { 0.f, 1.f, 0.f });
        blade = glm::translate(blade, { 1.0f, 0.f, 0.f });
        blade = glm::scale(blade, { 1.50f, 0.05f, 0.38f });
        drawCube(rr_sh, blade, glm::vec3(0.48f, 0.38f, 0.24f), texWood, 1.f);
    }
    m = glm::scale(glm::translate(glm::mat4(1.f), { fx, fY2 + 1.06f, fz }), { 0.05f, 0.18f, 0.05f });
    drawCylinder(rr_sh, m, glm::vec3(0.50f, 0.50f, 0.52f));
}

static void rrCeilingFans() {
    drawOneFan(-2.f, -1.f);
    drawOneFan(5.f, 2.f);
}

// ══════════════════════════════════════════════════════════════════════════
//  WALL CLOCK
// ══════════════════════════════════════════════════════════════════════════
static void rrWallClock() {
    float t = (float)glfwGetTime();
    float cx = 7.0f, cy = 3.8f, cz = RZ1 + 0.14f;

    col(0.22f, 0.18f, 0.12f);
    B(cx, cy, cz, 0.07f, 0.78f, 0.78f);
    col(0.97f, 0.96f, 0.94f);
    B(cx, cy, cz + 0.04f, 0.05f, 0.66f, 0.66f);

    col(0.30f, 0.26f, 0.22f);
    for (int h = 0; h < 12; h++) {
        float a = h * 30.f * 3.14159f / 180.f;
        B(cx + 0.04f, cy + cosf(a) * 0.25f, cz + sinf(a) * 0.25f + 0.04f, 0.03f, 0.035f, 0.03f);
    }
    col(0.18f, 0.14f, 0.10f);
    {
        float ang = -t * 3.f * 3.14159f / 180.f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { cx + 0.05f,cy,cz + 0.06f });
        m = glm::rotate(m, ang, { 1.f,0.f,0.f });
        m = glm::translate(m, { 0.f,0.09f,0.f });
        m = glm::scale(m, { 0.035f,0.20f,0.035f });
        setModel(m); renderCube();
    }
    {
        float ang = -t * 36.f * 3.14159f / 180.f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { cx + 0.05f,cy,cz + 0.06f });
        m = glm::rotate(m, ang, { 1.f,0.f,0.f });
        m = glm::translate(m, { 0.f,0.13f,0.f });
        m = glm::scale(m, { 0.025f,0.26f,0.025f });
        setModel(m); renderCube();
    }
    col(0.70f, 0.14f, 0.12f);
    B(cx + 0.06f, cy, cz + 0.07f, 0.045f, 0.045f, 0.045f);
}

// ══════════════════════════════════════════════════════════════════════════
//  LIBRARIAN DESK
// ══════════════════════════════════════════════════════════════════════════
static void rrReceptionDesk() {
    float dx = -1.0f, dz = RZ2 - 2.2f;

    col(0.36f, 0.28f, 0.16f);
    B(dx, 1.18f, dz, 3.00f, 2.36f, 0.95f);
    col(0.90f, 0.90f, 0.89f);
    B(dx, 2.40f, dz, 3.04f, 0.08f, 0.99f);
    col(0.40f, 0.31f, 0.18f);
    B(dx, 1.68f, dz + 0.46f, 3.00f, 0.76f, 0.09f);

    col(0.22f, 0.22f, 0.24f);
    B(dx + 0.4f, 2.84f, dz - 0.12f, 0.58f, 0.40f, 0.055f);
    col(0.18f, 0.44f, 0.82f);
    B(dx + 0.4f, 2.84f, dz - 0.10f, 0.48f, 0.30f, 0.04f);
    col(0.24f, 0.22f, 0.20f);
    B(dx + 0.4f, 2.48f, dz - 0.07f, 0.12f, 0.055f, 0.18f);

    drawOneChair(dx - 0.4f, dz - 0.95f, 0.f);
}

// ══════════════════════════════════════════════════════════════════════════
//  CORNER PLANTS
// ══════════════════════════════════════════════════════════════════════════
static void rrPlants() {
    auto plant = [&](float px, float pz) {
        col(0.64f, 0.30f, 0.14f);
        B(px, 0.60f, pz, 0.42f, 0.80f, 0.42f);
        col(0.24f, 0.16f, 0.10f);
        B(px, 1.02f, pz, 0.38f, 0.055f, 0.38f);
        for (int i = 0; i < 5; i++) {
            float a = i * 72.f * 3.14159f / 180.f;
            float ox = cosf(a) * 0.12f, oz = sinf(a) * 0.12f;
            col(0.20f, 0.52f, 0.22f);
            B(px + ox, 1.38f, pz + oz, 0.075f, 0.60f, 0.075f);
            col(0.24f, 0.58f, 0.26f);
            B(px + ox * 1.5f, 1.76f, pz + oz * 1.5f, 0.20f, 0.16f, 0.20f);
        }
        };
    plant(RX2 - 1.2f, RZ2 - 1.2f);
    plant(RX1 + 1.2f, RZ2 - 1.2f);
    plant(RX2 - 1.2f, RZ1 + 1.2f);
}

// ══════════════════════════════════════════════════════════════════════════
//  PUBLIC API
// ══════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────
//  RECOMMENDED CAMERA SETUP FOR TOP-DOWN VIEW (put this in your main.cpp):
//
//    camera.Position = glm::vec3(0.f, 20.f, 0.f);
//    camera.Pitch    = -89.0f;
//    camera.Yaw      = -90.0f;
//
//    glm::mat4 projection = glm::perspective(
//        glm::radians(60.0f),
//        (float)SCR_WIDTH / SCR_HEIGHT,
//        0.1f, 100.0f);
// ─────────────────────────────────────────────────────────────────────────

void drawReadingRoom() {
    GLint cur = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
    rr_sh = (unsigned int)cur;

    rrLighting();

    rrShell();
    rrLeftWindows();
    rrRightShelves();
    rrBackShelves();
    rrTableGrid();
    rrCeilingLights();
    rrCeilingFans();
    rrWallClock();
    rrReceptionDesk();
    rrPlants();
}

void updateReadingRoomFan() {
    rrFanAngle += 1.8f;
    if (rrFanAngle >= 360.f) rrFanAngle -= 360.f;
}

void toggleReadingRoomLight() {
    rrLightOn = !rrLightOn;
}