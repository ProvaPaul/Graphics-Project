#include "globals.h"
#include "draw_helpers.h"
#include "scene_ground.h"
#include "scene_cricket.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

void renderGround(unsigned int sh) {
    // Grass ground
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,-0.05f,0.f }), { 300.f,0.1f,300.f });
    drawCube(sh, m, COL_GRASS, texGrass, 30.f);
    // Front stone plaza
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,0.01f,25.f }), { 55.f,0.08f,35.f });
    drawCube(sh, m, COL_PLAZA, texTile, 8.f);
    // Inner courtyard paving
    m = glm::scale(glm::translate(glm::mat4(1.f), { -20.f,0.01f,-12.f }), { 20.f,0.08f,16.f });
    drawCube(sh, m, COL_PLAZA, texTile, 4.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 10.f,0.01f,-12.f }), { 20.f,0.08f,16.f });
    drawCube(sh, m, COL_PLAZA, texTile, 4.f);
    // Path connecting front to corridor
    m = glm::scale(glm::translate(glm::mat4(1.f), { -5.f,0.02f,0.f }), { 8.f,0.08f,20.f });
    drawCube(sh, m, COL_PLAZA, texTile, 3.f);
}
void renderBoundaryWall(unsigned int sh) {
    glm::mat4 m;
    const float WH = 2.5f, WT = 0.5f;
    const float GATE_CX = -5.f;  // Gate center aligned with building center
    const float GATE_HALF = 5.f; // Gate half-width (total 10m)
    // Front wall with centered gate gap (continuous up to both side walls)
    // Left segment:  x = [-33, -10]
    m = glm::scale(glm::translate(glm::mat4(1.f), { -21.5f, WH * 0.5f, 40.f }), { 23.f, WH, WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);
    // Right segment: x = [0, 33]
    m = glm::scale(glm::translate(glm::mat4(1.f), { 16.5f, WH * 0.5f, 40.f }), { 33.f, WH, WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);
    // Gate pillars - symmetric around GATE_CX
    m = glm::scale(glm::translate(glm::mat4(1.f), { GATE_CX - GATE_HALF, 1.5f, 40.f }), { 1.5f, 3.f, 1.5f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { GATE_CX + GATE_HALF, 1.5f, 40.f }), { 1.5f, 3.f, 1.5f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    // Pillar tops (sphere)
    drawSphere(sh, glm::scale(glm::translate(glm::mat4(1.f), { GATE_CX - GATE_HALF, 3.2f, 40.f }), { 1.8f, 1.8f, 1.8f }), COL_MARBLE, texMarble, 1.f);
    drawSphere(sh, glm::scale(glm::translate(glm::mat4(1.f), { GATE_CX + GATE_HALF, 3.2f, 40.f }), { 1.8f, 1.8f, 1.8f }), COL_MARBLE, texMarble, 1.f);
    // Gate opening is open - no arch between pillars
    // Left campus wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { -33.f,WH * 0.5f,-5.f }), { WT,WH,90.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);
    // Right campus wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 33.f,WH * 0.5f,-5.f }), { WT,WH,90.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);
    // Back wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 0.f,WH * 0.5f,-50.f }), { 66.f,WH,WT });
    drawCube(sh, m, COL_REDBRICK, texBrick, 8.f);
}

void renderPaths(unsigned int sh) {

    // ── Path colours & dimensions ─────────────────────────────────────────
    const glm::vec3 TILE_COL(0.82f, 0.82f, 0.80f);   // light-gray concrete tile
    const glm::vec3 GROUT_COL(0.60f, 0.60f, 0.58f);   // slightly darker grout border
    const float     TILE_H = 0.018f;                 // tile slab height (≈2 cm)
    const float     TILE_Y = 0.022f;                 // world Y (just above ground 0)
    const float     TILE_SZ = 1.20f;                  // tile face size (1.2 × 1.2 m)
    const float     GAP = 0.08f;                  // grout gap between tiles
    const float     STRIDE = TILE_SZ + GAP;          // centre-to-centre spacing
    const float     PATH_CX = -5.f;  // All paths aligned to building center

    // Helper: lay a rectangular grid of tiles centred at (cx, TILE_Y, cz)
    // nX = number of tiles along X,  nZ = number of tiles along Z
    // rotY = optional yaw rotation for diagonal paths (degrees, default 0)
    auto layTiles = [&](float cx, float cz, int nX, int nZ, float rotYdeg = 0.f)
        {
            // Precompute grid origin (top-left corner of grid in tile-local space)
            float ox = -(nX - 1) * 0.5f * STRIDE;
            float oz = -(nZ - 1) * 0.5f * STRIDE;

            for (int iz = 0; iz < nZ; iz++) {
                for (int ix = 0; ix < nX; ix++) {
                    // Local tile centre
                    float lx = ox + ix * STRIDE;
                    float lz = oz + iz * STRIDE;

                    // Rotate in XZ if needed (for diagonal branches)
                    float wx = lx, wz = lz;
                    if (rotYdeg != 0.f) {
                        float rad = glm::radians(rotYdeg);
                        wx = lx * cosf(rad) + lz * sinf(rad);
                        wz = -lx * sinf(rad) + lz * cosf(rad);
                    }

                    // World position
                    float tx = cx + wx;
                    float tz = cz + wz;

                    // ── Tile slab ──
                    glm::mat4 m = glm::scale(
                        glm::translate(glm::mat4(1.f), { tx, TILE_Y, tz }),
                        { TILE_SZ, TILE_H, TILE_SZ });
                    drawCube(sh, m, TILE_COL, texTile, 1.f);

                    // ── Grout border (slightly larger flat cube beneath tile) ──
                    m = glm::scale(
                        glm::translate(glm::mat4(1.f),
                            { tx, TILE_Y - TILE_H * 0.6f, tz }),
                        { TILE_SZ + GAP, TILE_H * 0.5f, TILE_SZ + GAP });
                    drawCube(sh, m, GROUT_COL);
                }
            }
        };

    // ── 1. Front entry path: gate (z≈40) down to Block-A porch (z≈8) ───
    //    Width = 6 tiles (≈7.8 m), centered with building
    layTiles(PATH_CX, 24.f, 6, 21);

    // ── 2. Central spine: Block-A back (z≈-1) → Block-B front (z≈-24) ──
    //    Width = 3 tiles,  Length ≈ 23 units → ~19 tiles
    layTiles(PATH_CX, -12.f, 3, 19);   // z: -21 → -3

    // ── 3. Left lateral branch: spine → badminton area (x: -5 → -50) ───
    //    Width = 2 tiles,  Length ≈ 45 units → ~36 tiles  (oriented along X)
    //    We rotate 90° so the tile grid runs along X instead of Z
    layTiles(-27.f, -10.f, 36, 2, 0.f);   // already along X layout

    // ── 4. Right lateral branch: spine → cricket / admin area (x: -5 → +50) ─
    layTiles(22.f, -10.f, 36, 2, 0.f);

    // ── 5. Short connector path: front plaza → central spine ─────────────
    //    Bridges the gap between plaza edge (z≈8) and spine top (z≈3)
    layTiles(PATH_CX, 2.f, 3, 9);

    // ── 6. Side path from admin annex to right branch (diagonal flair) ───
    layTiles(12.f, 3.f, 2, 8);    // small path toward annex door

    // ── 7. Courtyard connecting tiles (left & right of central corridor) ─
    layTiles(-16.f, -12.f, 6, 4);   // left courtyard tiles
    layTiles(6.f, -12.f, 6, 4);   // right courtyard tiles

    // ── 8. Decorative step tiles at Block-A entrance ───────────────────
    //    Five raised steps (each 0.08 m higher than the previous)
    for (int step = 0; step < 5; step++) {
        float stepY = TILE_Y + step * 0.08f;
        float stepZ = 9.5f - step * 0.50f;
        float stepW = 8.0f - step * 0.4f;
        glm::mat4 m = glm::scale(
            glm::translate(glm::mat4(1.f), { PATH_CX, stepY, stepZ }),
            { stepW, TILE_H + 0.06f, 0.55f });
        drawCube(sh, m, glm::vec3(0.78f + step * 0.02f, 0.78f + step * 0.02f,
            0.76f + step * 0.02f), texTile, 2.f);
    }

    // ── 9. Landing platform at Block-A entrance ──────────────────────────
    glm::mat4 m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX, TILE_Y + 0.02f, 10.5f }),
        { 6.5f, TILE_H + 0.04f, 1.5f });
    drawCube(sh, m, TILE_COL, texTile, 2.f);

    // ── 10. Matching entrance steps at Block-B north face ────────────────
    for (int step = 0; step < 4; step++) {
        float stepY = TILE_Y + step * 0.08f;
        float stepZ = -24.f + step * 0.50f;
        float stepW = 8.0f - step * 0.4f;
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { PATH_CX, stepY, stepZ }),
            { stepW, TILE_H + 0.06f, 0.55f });
        drawCube(sh, m, glm::vec3(0.78f + step * 0.02f, 0.78f + step * 0.02f,
            0.76f + step * 0.02f), texTile, 2.f);
    }

    // ── 11. Block-B landing platform ─────────────────────────────────────
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX, TILE_Y + 0.02f, -23.5f }),
        { 6.5f, TILE_H + 0.04f, 1.5f });
    drawCube(sh, m, TILE_COL, texTile, 2.f);

    // ── 12. Raised kerb edges along the central spine (both sides) ────────
    const glm::vec3 KERB(0.55f, 0.55f, 0.53f);
    // Left kerb
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX - 2.0f, TILE_Y + 0.06f, -12.f }),
        { 0.22f, 0.12f, 23.f });
    drawCube(sh, m, KERB, texConcrete, 4.f);
    // Right kerb
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX + 2.0f, TILE_Y + 0.06f, -12.f }),
        { 0.22f, 0.12f, 23.f });
    drawCube(sh, m, KERB, texConcrete, 4.f);

    // Front path kerbs (wider to match expanded path)
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX - 4.0f, TILE_Y + 0.06f, 24.f }),
        { 0.22f, 0.12f, 21.f });
    drawCube(sh, m, KERB, texConcrete, 3.f);
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { PATH_CX + 4.0f, TILE_Y + 0.06f, 24.f }),
        { 0.22f, 0.12f, 21.f });
    drawCube(sh, m, KERB, texConcrete, 3.f);

    // ── 13. Decorative lamp posts along front path ───────────────────────
    const glm::vec3 LAMP_POST(0.35f, 0.35f, 0.38f);
    const glm::vec3 LAMP_GLOW(1.0f, 0.95f, 0.80f);
    for (int i = 0; i < 4; i++) {
        float lx = PATH_CX + (i % 2 == 0 ? -4.5f : 4.5f);
        float lz = 18.f + i * 7.f;
        // Post
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { lx, 2.0f, lz }),
            { 0.12f, 4.0f, 0.12f });
        drawCube(sh, m, LAMP_POST, texConcrete, 1.f);
        // Lamp head
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { lx, 4.2f, lz }),
            { 0.5f, 0.3f, 0.5f });
        drawCube(sh, m, LAMP_GLOW, texConcrete, 1.f);
    }
}

