#include "globals.h"
#include "draw_helpers.h"
#include "scene_buildings.h"
#include "scene_props.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

// ── DORMITORY BLOCK A (front building – 5 floors) ────────────────────────────
void renderDormBlockA(unsigned int sh) {
    const float AX = -5.f, AZ = 5.f;
    const float ALEN = 40.f;           // X span
    const float ADEP = 12.f;           // Z depth
    const float FLOOR_H = 3.2f;
    const int   NFLOOR = 5;
    const float TOTAL_H = FLOOR_H * NFLOOR;
    const float ZN = AZ + ADEP * 0.5f;  // north face z
    const float ZS = AZ - ADEP * 0.5f;  // south face z

    glm::mat4 m;

    // ── Core structure ──
    // Side walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX - ALEN * 0.5f - 0.2f, TOTAL_H * 0.5f, AZ }), { 0.4f,TOTAL_H,ADEP });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX + ALEN * 0.5f + 0.2f, TOTAL_H * 0.5f, AZ }), { 0.4f,TOTAL_H,ADEP });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);

    // Roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, TOTAL_H + 0.2f, AZ }), { ALEN + 1.5f,0.5f,ADEP + 1.f });
    drawCube(sh, m, COL_ROOF, texConcrete, 5.f);
    // Roof parapet
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, TOTAL_H + 0.7f, ZN }), { ALEN + 1.5f,0.9f,0.35f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 4.f);

    // Ground floor plinth
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, 0.3f, AZ }), { ALEN + 2.f,0.6f,ADEP + 1.5f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);
    // Floor slab
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, 0.05f, AZ }), { ALEN,0.1f,ADEP });
    drawCube(sh, m, COL_FLOOR, texTile, 6.f);

    // ── Per-floor slabs + balconies (front face) ──
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        // Floor slab divider
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX, fBase + FLOOR_H - 0.15f, AZ }), { ALEN + 0.5f,0.3f,ADEP + 0.5f });
        drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);

        // Balcony slab on front face per floor (floors 1-4)
        if (fl > 0) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { AX, fBase + 0.12f, ZN + 0.8f }), { ALEN,0.25f,1.6f });
            drawCube(sh, m, COL_BALCONY, texConcrete, 3.f);
            // Railing posts
            for (int ri = 0; ri <= 10; ri++) {
                float rx = AX - ALEN * 0.5f + ri * (ALEN / 10.f);
                m = glm::scale(glm::translate(glm::mat4(1.f), { rx, fBase + 0.8f, ZN + 1.5f }), { 0.08f,1.3f,0.08f });
                drawCube(sh, m, COL_RAILING);
            }
            // Top rail bar
            m = glm::scale(glm::translate(glm::mat4(1.f), { AX, fBase + 1.45f, ZN + 1.5f }), { ALEN,0.08f,0.08f });
            drawCube(sh, m, COL_RAILING);
        }
    }

    // ── Front face columns & windows ──
    for (int i = 0; i < 11; i++) {
        float x = AX - ALEN * 0.5f + i * (ALEN / 10.f);
        // Column full height
        glm::mat4 cyl = glm::translate(glm::mat4(1.f), { x, TOTAL_H * 0.5f, ZN });
        cyl = glm::scale(cyl, { 0.50f,TOTAL_H,0.50f });
        drawCylinder(sh, cyl, COL_COLUMN, texConcrete, 3.f);
        // Capital
        m = glm::scale(glm::translate(glm::mat4(1.f), { x,TOTAL_H + 0.05f,ZN }), { 0.65f,0.4f,0.65f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    }
    // Windows per floor on front face
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        for (int i = 0; i < 10; i++) {
            float x = AX - ALEN * 0.5f + 0.5f * (ALEN / 10.f) + i * (ALEN / 10.f);
            // Window glass
            m = glm::scale(glm::translate(glm::mat4(1.f), { x, fBase + FLOOR_H * 0.55f, ZN }), { 2.8f,2.0f,0.28f });
            drawCrystalGlass(sh, m);
            // Window sill
            m = glm::scale(glm::translate(glm::mat4(1.f), { x, fBase + FLOOR_H * 0.55f - 1.1f, ZN + 0.15f }), { 3.0f,0.18f,0.3f });
            drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
        }
    }

    // ── Back face (south) solid wall with small windows ──
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, TOTAL_H * 0.5f, ZS }), { ALEN,TOTAL_H,0.4f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        for (int i = 0; i < 6; i++) {
            float x = AX - 18.f + i * 7.2f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { x, fBase + FLOOR_H * 0.55f, ZS }), { 2.2f,1.5f,0.28f });
            drawGlass(sh, m);
        }
    }

    // ── Ground-floor main entrance (front) ──
    // Door frame - symmetric around AX (centered single large door)
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, FLOOR_H * 0.42f, ZN + 0.05f }), { 4.2f, FLOOR_H * 0.85f, 0.4f });
    drawCube(sh, m, COL_DOOR, texWood, 1.f);
    // Door transom glass
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, FLOOR_H * 0.87f, ZN + 0.05f }), { 4.2f, FLOOR_H * 0.28f, 0.28f });
    drawGlass(sh, m);

    // ── Entrance canopy / porch ──
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, FLOOR_H + 0.15f, ZN + 2.5f }), { 10.f, 0.35f, 5.5f });
    drawCube(sh, m, COL_ROOF, texConcrete, 2.f);
    // Canopy trim
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, FLOOR_H + 0.38f, ZN + 2.5f }), { 10.4f, 0.12f, 5.9f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    // Canopy support columns - symmetric
    for (int s : {-1, 1}) {
        glm::mat4 cc = glm::translate(glm::mat4(1.f), { AX + s * 4.8f, FLOOR_H * 0.5f, ZN + 4.5f });
        cc = glm::scale(cc, { 0.5f, FLOOR_H, 0.5f });
        drawCylinder(sh, cc, COL_COLUMN, texConcrete, 2.f);
        // Column base
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX + s * 4.8f, 0.3f, ZN + 4.5f }), { 0.7f, 0.6f, 0.7f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
        // Column capital
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX + s * 4.8f, FLOOR_H + 0.05f, ZN + 4.5f }), { 0.65f, 0.4f, 0.65f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    }

    // ── Symmetric entrance decorations ──
    // Decorative urns on canopy
    for (int s : {-1, 1}) {
        glm::mat4 urn = glm::translate(glm::mat4(1.f), { AX + s * 4.8f, FLOOR_H + 0.5f, ZN + 4.5f });
        urn = glm::scale(urn, { 0.3f, 0.4f, 0.3f });
        drawSphere(sh, urn, COL_MARBLE);
    }

    // ── Entrance steps landing ──
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX, 0.05f, ZN + 5.5f }), { 7.f, 0.1f, 2.f });
    drawCube(sh, m, COL_FLOOR, texTile, 2.f);

    // ── Symmetric decorative trees near entrance ──
    auto drawTree = [&](float tx, float tz) {
        // Planter box
        m = glm::scale(glm::translate(glm::mat4(1.f), { tx, 0.4f, tz }), { 1.2f, 0.8f, 1.2f });
        drawCube(sh, m, COL_DOOR, texWood, 1.f);
        // Tree trunk
        glm::mat4 trunk = glm::translate(glm::mat4(1.f), { tx, 1.5f, tz });
        trunk = glm::scale(trunk, { 0.25f, 2.0f, 0.25f });
        drawCylinder(sh, trunk, COL_TRUNK);
        // Tree foliage (layered spheres)
        glm::mat4 foliage = glm::translate(glm::mat4(1.f), { tx, 2.8f, tz });
        foliage = glm::scale(foliage, { 1.2f, 1.0f, 1.2f });
        drawSphere(sh, foliage, COL_PALM);
        foliage = glm::translate(glm::mat4(1.f), { tx, 3.4f, tz });
        foliage = glm::scale(foliage, { 0.9f, 0.8f, 0.9f });
        drawSphere(sh, foliage, COL_PALM);
    };
    // Place trees symmetrically on both sides of entrance
    drawTree(AX - 8.f, ZN + 3.f);
    drawTree(AX + 8.f, ZN + 3.f);

    // ── Decorative wall sconces near entrance ──
    for (int s : {-1, 1}) {
        // Sconce plate
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX + s * 2.5f, FLOOR_H * 0.7f, ZN + 0.1f }), { 0.6f, 0.8f, 0.15f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
        // Sconce light
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX + s * 2.5f, FLOOR_H * 0.6f, ZN + 0.15f }), { 0.3f, 0.3f, 0.2f });
        drawCube(sh, m, glm::vec3(1.0f, 0.95f, 0.8f));
    }

    // ── Staircase block on right side ──
    m = glm::scale(glm::translate(glm::mat4(1.f), { AX + ALEN * 0.5f - 3.f, TOTAL_H * 0.5f, AZ }), { 6.f,TOTAL_H,ADEP });
    drawCube(sh, m, glm::vec3(0.80f, 0.80f, 0.78f), texConcrete, 3.f);
    // Staircase windows
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        m = glm::scale(glm::translate(glm::mat4(1.f), { AX + ALEN * 0.5f - 3.f, fBase + FLOOR_H * 0.55f, ZN + 0.05f }), { 3.f,1.8f,0.28f });
        drawCrystalGlass(sh, m);
    }
}

// ── DORMITORY BLOCK B (back building – identical style) ──────────────────────
void renderDormBlockB(unsigned int sh) {
    const float BX = -5.f, BZ = -30.f;
    const float BLEN = 40.f;
    const float BDEP = 12.f;
    const float FLOOR_H = 3.2f;
    const int   NFLOOR = 4;             // Block B is 4 floors
    const float TOTAL_H = FLOOR_H * NFLOOR;
    const float ZN = BZ + BDEP * 0.5f;
    const float ZS = BZ - BDEP * 0.5f;

    glm::mat4 m;

    // Side walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX - BLEN * 0.5f - 0.2f, TOTAL_H * 0.5f, BZ }), { 0.4f,TOTAL_H,BDEP });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX + BLEN * 0.5f + 0.2f, TOTAL_H * 0.5f, BZ }), { 0.4f,TOTAL_H,BDEP });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);
    // Roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX,TOTAL_H + 0.2f,BZ }), { BLEN + 1.5f,0.5f,BDEP + 1.f });
    drawCube(sh, m, COL_ROOF, texConcrete, 5.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX,TOTAL_H + 0.7f,ZN }), { BLEN + 1.5f,0.9f,0.35f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 4.f);
    // Plinth & floor
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX,0.3f,BZ }), { BLEN + 2.f,0.6f,BDEP + 1.5f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);

    // Per-floor slabs
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        m = glm::scale(glm::translate(glm::mat4(1.f), { BX,fBase + FLOOR_H - 0.15f,BZ }), { BLEN + 0.5f,0.3f,BDEP + 0.5f });
        drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);
        if (fl > 0) {
            // North face balcony
            m = glm::scale(glm::translate(glm::mat4(1.f), { BX,fBase + 0.12f,ZN + 0.8f }), { BLEN,0.25f,1.6f });
            drawCube(sh, m, COL_BALCONY, texConcrete, 3.f);
            for (int ri = 0; ri <= 10; ri++) {
                float rx = BX - BLEN * 0.5f + ri * (BLEN / 10.f);
                m = glm::scale(glm::translate(glm::mat4(1.f), { rx,fBase + 0.8f,ZN + 1.5f }), { 0.08f,1.3f,0.08f });
                drawCube(sh, m, COL_RAILING);
            }
            m = glm::scale(glm::translate(glm::mat4(1.f), { BX,fBase + 1.45f,ZN + 1.5f }), { BLEN,0.08f,0.08f });
            drawCube(sh, m, COL_RAILING);
        }
    }

    // Front (north) face columns & windows
    for (int i = 0; i < 11; i++) {
        float x = BX - BLEN * 0.5f + i * (BLEN / 10.f);
        glm::mat4 cyl = glm::translate(glm::mat4(1.f), { x,TOTAL_H * 0.5f,ZN });
        cyl = glm::scale(cyl, { 0.50f,TOTAL_H,0.50f });
        drawCylinder(sh, cyl, COL_COLUMN, texConcrete, 3.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { x,TOTAL_H + 0.05f,ZN }), { 0.65f,0.4f,0.65f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    }
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        for (int i = 0; i < 10; i++) {
            float x = BX - BLEN * 0.5f + 0.5f * (BLEN / 10.f) + i * (BLEN / 10.f);
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,fBase + FLOOR_H * 0.55f,ZN }), { 2.8f,2.0f,0.28f });
            drawCrystalGlass(sh, m);
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,fBase + FLOOR_H * 0.55f - 1.1f,ZN + 0.15f }), { 3.0f,0.18f,0.3f });
            drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
        }
    }

    // Back (south) solid wall with ventilation windows
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX,TOTAL_H * 0.5f,ZS }), { BLEN,TOTAL_H,0.4f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);
    for (int fl = 0; fl < NFLOOR; fl++) {
        float fBase = fl * FLOOR_H;
        for (int i = 0; i < 5; i++) {
            float x = BX - 16.f + i * 8.f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,fBase + FLOOR_H * 0.55f,ZS }), { 2.2f,1.5f,0.28f });
            drawGlass(sh, m);
        }
    }

    // Entrance doors north face
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX - 1.1f,FLOOR_H * 0.42f,ZN + 0.05f }), { 2.2f,FLOOR_H * 0.85f,0.4f });
    drawCube(sh, m, COL_DOOR, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { BX + 1.1f,FLOOR_H * 0.42f,ZN + 0.05f }), { 2.2f,FLOOR_H * 0.85f,0.4f });
    drawCube(sh, m, COL_DOOR, texWood, 1.f);
}

// ── CONNECTING CORRIDOR (between Block A and B, Z-axis oriented) ──────────────
void renderCorridor(unsigned int sh) {
    const float MX = 15.f;
    const float MZ = -13.f;
    const float XWID = 10.f;
    const float ZDEP = 42.f;
    const float ZN = MZ + ZDEP * 0.5f;
    const float ZS = MZ - ZDEP * 0.5f;
    const float STEP = ZDEP / 10.f;

    glm::mat4 m;

    // North & South end walls
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,4.f,ZN }), { XWID,8.f,0.4f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,4.f,ZS }), { XWID,8.f,0.4f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 2.f);

    // Roof with skylight strip
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,8.2f,MZ }), { XWID + 1.f,0.4f,ZDEP + 1.f });
    drawCube(sh, m, COL_ROOF, texConcrete, 6.f);
    // Skylight glass strip on roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,8.45f,MZ }), { XWID * 0.5f,0.1f,ZDEP * 0.6f });
    drawCrystalGlass(sh, m);

    // Floor & plinth
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,0.05f,MZ }), { XWID,0.1f,ZDEP });
    drawCube(sh, m, COL_FLOOR, texTile, 8.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX,0.3f,MZ }), { XWID + 1.f,0.6f,ZDEP + 1.f });
    drawCube(sh, m, COL_CONCRETE, texConcrete, 5.f);

    // Left (west) face columns
    for (int i = 0; i < 11; i++) {
        float z = ZN - i * STEP;
        glm::mat4 cyl = glm::translate(glm::mat4(1.f), { MX - XWID * 0.5f,4.f,z });
        cyl = glm::scale(cyl, { 0.45f,8.f,0.45f });
        drawCylinder(sh, cyl, COL_COLUMN, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { MX - XWID * 0.5f,8.f,z }), { 0.65f,0.4f,0.65f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    }
    // Left glass panels
    for (int i = 0; i < 10; i++) {
        float z = ZN - STEP * 0.5f - i * STEP;
        m = glm::scale(glm::translate(glm::mat4(1.f), { MX - XWID * 0.5f,4.f,z }), { 0.3f,7.5f,STEP * 0.75f });
        drawGlass(sh, m);
    }

    // Right (east) face columns
    for (int i = 0; i < 11; i++) {
        float z = ZN - i * STEP;
        glm::mat4 cyl = glm::translate(glm::mat4(1.f), { MX + XWID * 0.5f,4.f,z });
        cyl = glm::scale(cyl, { 0.45f,8.f,0.45f });
        drawCylinder(sh, cyl, COL_COLUMN, texConcrete, 2.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { MX + XWID * 0.5f,8.f,z }), { 0.65f,0.4f,0.65f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);
    }
    // Right glass panels (skip door gap)
    for (int i = 0; i < 10; i++) {
        float z = ZN - STEP * 0.5f - i * STEP;
        if (z > -11.f && z < -15.f) continue;
        m = glm::scale(glm::translate(glm::mat4(1.f), { MX + XWID * 0.5f,4.f,z }), { 0.3f,7.5f,STEP * 0.75f });
        drawGlass(sh, m);
    }

    // CAFETERIA sign on east face
    {
        const float SX = MX + XWID * 0.5f + 0.22f;
        const float LW = 0.38f, LH = 0.62f, LD = 0.15f, SP = 0.42f;
        float lz = -20.6f, ly = 5.5f;
        auto be = [&](float oz, float oy, float sw, float sh2) {
            glm::mat4 mm = glm::translate(glm::mat4(1.f), { SX, ly + oy, lz - oz });
            mm = glm::scale(mm, { LD,sh2,sw });
            drawSignCube(sh, mm);
            };
        // C
        be(.07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW, .13f); be(LW * .5f, -LH * .42f, LW, .13f); lz -= SP;
        // A
        be(.07f, 0.f, .13f, LH); be(LW - .07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW, .13f); be(LW * .5f, 0.f, LW, .13f); lz -= SP;
        // F
        be(.07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW, .13f); be(LW * .45f, 0.f, LW * .85f, .13f); lz -= SP;
        // E
        be(.07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW, .13f); be(LW * .45f, 0.f, LW * .85f, .13f); be(LW * .5f, -LH * .42f, LW, .13f); lz -= SP;
        // T
        be(LW * .5f, LH * .42f, LW, .13f); be(LW * .5f, 0.f, .13f, LH); lz -= SP;
        // E
        be(.07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW, .13f); be(LW * .45f, 0.f, LW * .85f, .13f); be(LW * .5f, -LH * .42f, LW, .13f); lz -= SP;
        // R
        be(.07f, 0.f, .13f, LH); be(LW * .5f, LH * .42f, LW * .7f, .13f); be(LW - .07f, LH * .2f, .13f, LH * .5f); be(LW * .5f, 0.f, LW * .85f, .13f); be(LW - .07f, -LH * .22f, .13f, LH * .38f); lz -= SP;
        // I
        be(LW * .5f, 0.f, .13f, LH);
    }
    // Cafeteria door on east face
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX + XWID * 0.5f + 0.16f,3.f,-13.f }), { 0.15f,6.f,5.8f });
    drawCube(sh, m, COL_DOOR);

    // Entrance doors north end
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX - 0.9f,3.f,ZN + 0.1f }), { 1.8f,6.f,0.15f });
    drawCube(sh, m, COL_DOOR);
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX + 0.9f,3.f,ZN + 0.1f }), { 1.8f,6.f,0.15f });
    drawCube(sh, m, COL_DOOR);
    // Columns – north end
    for (int i = 0; i < 5; i++) {
        float x = MX - XWID * 0.5f + i * (XWID / 4.f);
        glm::mat4 cm = glm::translate(glm::mat4(1.f), { x,4.f,ZN });
        cm = glm::rotate(cm, glm::radians(90.f), { 1,0,0 });
        cm = glm::scale(cm, { 0.35f,0.35f,8.f });
        drawCylinder(sh, cm, COL_COLUMN, texConcrete, 2.f);
    }

    // Entrance doors south end
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX - 0.9f,3.f,ZS - 0.1f }), { 1.8f,6.f,0.15f });
    drawCube(sh, m, COL_DOOR);
    m = glm::scale(glm::translate(glm::mat4(1.f), { MX + 0.9f,3.f,ZS - 0.1f }), { 1.8f,6.f,0.15f });
    drawCube(sh, m, COL_DOOR);
    // Columns – south end
    for (int i = 0; i < 5; i++) {
        float x = MX - XWID * 0.5f + i * (XWID / 4.f);
        glm::mat4 cm = glm::translate(glm::mat4(1.f), { x,4.f,ZS });
        cm = glm::rotate(cm, glm::radians(90.f), { 1,0,0 });
        cm = glm::scale(cm, { 0.35f,0.35f,8.f });
        drawCylinder(sh, cm, COL_COLUMN, texConcrete, 2.f);
    }
}
void renderAdminAnnex(unsigned int sh) {
    float cx = 20.f, cz = 3.f, radius = 7.f;
    int segs = 14;
    glm::mat4 m;

    // Curved brick wall segments
    for (int i = 0; i < segs; i++) {
        float t0 = glm::radians(-10.f + (float)i * (190.f / segs));
        float t1 = glm::radians(-10.f + (float)(i + 1) * (190.f / segs));
        float tm = (t0 + t1) * 0.5f;
        float wx = cx + radius * cosf(tm), wz = cz + radius * sinf(tm);
        float segLen = 2.f * radius * sinf((t1 - t0) * 0.5f) + 0.05f;
        m = glm::translate(glm::mat4(1.f), { wx,6.f,wz });
        m = glm::rotate(m, -tm + glm::radians(90.f), { 0,1,0 });
        m = glm::scale(m, { segLen,12.f,0.55f });
        drawCube(sh, m, COL_REDBRICK, texBrick, 2.f);
    }

    // Front flat wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 20.f,6.f,-4.f }), { 14.f,12.f,0.4f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 3.f);
    // Side wall
    m = glm::scale(glm::translate(glm::mat4(1.f), { 14.5f,6.f,-0.5f }), { 0.4f,12.f,7.5f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 2.f);
    // Flat roof
    m = glm::scale(glm::translate(glm::mat4(1.f), { 20.f,12.2f,1.f }), { 13.5f,0.4f,11.5f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 3.f);
    // Ground plinth
    m = glm::scale(glm::translate(glm::mat4(1.f), { 20.f,0.3f,1.f }), { 14.f,0.6f,12.f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 3.f);

    // Windows on front wall (annex)
    for (int fl = 0; fl < 3; fl++) {
        float fy = 2.f + fl * 3.8f;
        for (int i = 0; i < 3; i++) {
            float x = 15.5f + i * 3.f;
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,fy,-3.7f }), { 2.f,2.2f,0.12f });
            drawCrystalGlass(sh, m);
            // Window sill
            m = glm::scale(glm::translate(glm::mat4(1.f), { x,fy - 1.2f,-3.55f }), { 2.2f,0.15f,0.25f });
            drawCube(sh, m, COL_COLUMN);
        }
    }

    // ── STUDENT DORMITORY banner sign on front wall ──
    {
        const float SZ = 12.0f;
        const float LY = 12.5f;
        const float LETTER_H = 1.2f;
        const float LETTER_W = 0.8f;
        const float LETTER_D = 0.25f;
        const float SPACE = 0.9f;
        
        glm::mat4 m;
        const glm::vec3 SIGN_COLOR(1.0f, 0.95f, 0.8f);
        
        float lx = -7.0f;
        
        auto drawLetter = [&](char c) {
            switch(c) {
                case 'S': 
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY + LETTER_H*0.4f, SZ}), {LETTER_W, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY - LETTER_H*0.4f, SZ}), {LETTER_W, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'T':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.5f, LY, SZ}), {LETTER_W, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY + LETTER_H*0.4f, SZ}), {LETTER_W, LETTER_H*0.2f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'U':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY - LETTER_H*0.35f, SZ}), {LETTER_W*0.4f, LETTER_H*0.3f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'D':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.5f, LY + LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.5f, LY - LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY, SZ}), {LETTER_W*0.2f, LETTER_H*0.7f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'E':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY + LETTER_H*0.35f, SZ}), {LETTER_W, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.8f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY - LETTER_H*0.35f, SZ}), {LETTER_W, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'N':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY, SZ}), {LETTER_W*0.3f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'R':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.5f, LY + LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY + LETTER_H*0.1f, SZ}), {LETTER_W*0.2f, LETTER_H*0.4f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY - LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.3f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'M':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY + LETTER_H*0.3f, SZ}), {LETTER_W*0.3f, LETTER_H*0.4f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'I':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'O':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY, SZ}), {LETTER_W*0.2f, LETTER_H, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY + LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY - LETTER_H*0.35f, SZ}), {LETTER_W*0.5f, LETTER_H*0.25f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
                case 'Y':
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx, LY + LETTER_H*0.2f, SZ}), {LETTER_W*0.2f, LETTER_H*0.6f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.8f, LY + LETTER_H*0.2f, SZ}), {LETTER_W*0.2f, LETTER_H*0.6f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    m = glm::scale(glm::translate(glm::mat4(1.f), {lx + LETTER_W*0.4f, LY - LETTER_H*0.3f, SZ}), {LETTER_W*0.3f, LETTER_H*0.5f, LETTER_D}); drawCube(sh, m, SIGN_COLOR);
                    break;
            }
            lx += SPACE;
        };
        
        // STUDENT
        drawLetter('S'); drawLetter('T'); drawLetter('U'); drawLetter('D');
        drawLetter('E'); drawLetter('N'); drawLetter('T');
        
        lx += 0.5f;
        
        // DORMITORY
        drawLetter('D'); drawLetter('O'); drawLetter('R'); drawLetter('M');
        drawLetter('I'); drawLetter('T'); drawLetter('O'); drawLetter('R'); drawLetter('Y');
    }

    // Admin annex door
    m = glm::scale(glm::translate(glm::mat4(1.f), { 20.f,1.5f,-3.8f }), { 2.5f,3.f,0.25f });
    drawCube(sh, m, COL_DOOR, texWood, 1.f);
}
void renderRoomInterior(unsigned int sh) {

    static const glm::vec3 ZERO3(0.f, 0.f, 0.f);

    // ── Room geometry constants ───────────────────────────────────────────
    const float WALL_T = 0.30f;   // wall thickness
    const float CLG_H = 3.00f;   // interior ceiling height
    const float FLOOR_Y = 0.62f;   // interior floor Y (on top of ground-floor plinth)

    // Room 1 bounds (left room, camera enters here)
    const float R1_X1 = -14.5f, R1_X2 = -1.5f;   // west / east
    const float R1_Z1 = -0.5f, R1_Z2 = 9.5f;   // south / north
    const float R1_XC = (R1_X1 + R1_X2) * 0.5f;
    const float R1_ZC = (R1_Z1 + R1_Z2) * 0.5f;
    const float R1_W = R1_X2 - R1_X1;
    const float R1_D = R1_Z2 - R1_Z1;

    // Room 2 bounds (right room)
    const float R2_X1 = -1.5f, R2_X2 = 11.5f;
    const float R2_Z1 = -0.5f, R2_Z2 = 9.5f;
    const float R2_XC = (R2_X1 + R2_X2) * 0.5f;
    const float R2_ZC = (R2_Z1 + R2_Z2) * 0.5f;
    const float R2_W = R2_X2 - R2_X1;
    const float R2_D = R2_Z2 - R2_Z1;

    // ── Colours ───────────────────────────────────────────────────────────
    const glm::vec3 WALL_COL(0.92f, 0.90f, 0.86f);   // off-white plaster
    const glm::vec3 FLOOR_COL(0.78f, 0.74f, 0.68f);   // warm beige tile
    const glm::vec3 CLG_COL(0.95f, 0.94f, 0.92f);   // white ceiling
    const glm::vec3 BED_FRAME(0.38f, 0.26f, 0.14f);   // dark wood
    const glm::vec3 MATTRESS(0.88f, 0.84f, 0.78f);   // cream mattress
    const glm::vec3 PILLOW(0.96f, 0.94f, 0.90f);   // white pillow
    const glm::vec3 BLANKET(0.28f, 0.42f, 0.68f);   // blue blanket
    const glm::vec3 TABLE_COL(0.55f, 0.40f, 0.22f);   // medium wood
    const glm::vec3 CHAIR_COL(0.30f, 0.22f, 0.12f);   // dark wood
    const glm::vec3 SHELF_COL(0.48f, 0.35f, 0.18f);   // shelf wood
    const glm::vec3 BOOK_COLS[4] = {
        {0.72f,0.18f,0.18f}, {0.18f,0.38f,0.70f},
        {0.22f,0.58f,0.25f}, {0.72f,0.65f,0.18f}
    };
    const glm::vec3 WARD_COL(0.45f, 0.33f, 0.18f);   // wardrobe wood
    const glm::vec3 LAMP_COL(0.95f, 0.93f, 0.82f);   // warm lamp glow

    glm::mat4 m;

    // ──────────────────────────────────────────────────────────────────────
    // Helper lambda: build one complete room
    // ──────────────────────────────────────────────────────────────────────
    auto buildRoom = [&](float x1, float x2, float z1, float z2,
        float xc, float zc, float rw, float rd,
        bool  mirrorFurniture)   // mirror furniture X for room 2
        {
            // ── Floor ──────────────────────────────────────────────────────
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { xc, FLOOR_Y - 0.06f, zc }),
                { rw, 0.12f, rd });
            drawCube(sh, m, FLOOR_COL, texTile, 4.f);

            // ── Ceiling ────────────────────────────────────────────────────
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { xc, FLOOR_Y + CLG_H + 0.06f, zc }),
                { rw, 0.14f, rd });
            drawCube(sh, m, CLG_COL, texConcrete, 3.f);

            // ── South wall (z = z1, solid) ─────────────────────────────────
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H * 0.5f, z1 - WALL_T * 0.5f }),
                { rw + WALL_T, CLG_H, WALL_T });
            drawCube(sh, m, WALL_COL, texConcrete, 2.f);

            // Window recess in south wall (centred)
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H * 0.55f, z1 - WALL_T * 0.5f }),
                { rw * 0.38f, CLG_H * 0.38f, WALL_T + 0.04f });
            drawCrystalGlass(sh, m);
            // Window sill
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H * 0.36f, z1 - 0.04f }),
                { rw * 0.42f, 0.10f, 0.22f });
            drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);

            // ── North wall (z = z2) with doorway gap ───────────────────────
            // Left segment
            float doorW = 1.80f;
            float doorH = 2.40f;
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { x1 + (rw - doorW) * 0.25f, FLOOR_Y + CLG_H * 0.5f, z2 + WALL_T * 0.5f }),
                { (rw - doorW) * 0.5f, CLG_H, WALL_T });
            drawCube(sh, m, WALL_COL, texConcrete, 2.f);
            // Right segment
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { x2 - (rw - doorW) * 0.25f, FLOOR_Y + CLG_H * 0.5f, z2 + WALL_T * 0.5f }),
                { (rw - doorW) * 0.5f, CLG_H, WALL_T });
            drawCube(sh, m, WALL_COL, texConcrete, 2.f);
            // Transom above door
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + doorH + (CLG_H - doorH) * 0.5f, z2 + WALL_T * 0.5f }),
                { doorW, CLG_H - doorH, WALL_T });
            drawCube(sh, m, WALL_COL, texConcrete, 1.f);
            // Door frame (dark wood)
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + doorH * 0.5f, z2 + WALL_T * 0.5f }),
                { doorW, doorH, WALL_T + 0.04f });
            drawCube(sh, m, COL_DOOR, texWood, 1.f);

            // ── West wall (x = x1, solid) ──────────────────────────────────
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { x1 - WALL_T * 0.5f, FLOOR_Y + CLG_H * 0.5f, zc }),
                { WALL_T, CLG_H, rd });
            drawCube(sh, m, WALL_COL, texConcrete, 2.f);

            // ── East wall (x = x2, solid) ──────────────────────────────────
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { x2 + WALL_T * 0.5f, FLOOR_Y + CLG_H * 0.5f, zc }),
                { WALL_T, CLG_H, rd });
            drawCube(sh, m, WALL_COL, texConcrete, 2.f);

            // ── Skirting boards ────────────────────────────────────────────
            glm::vec3 skirt(0.65f, 0.55f, 0.40f);
            // South
            m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FLOOR_Y + 0.08f,z1 }), { rw,0.16f,0.06f });
            drawCube(sh, m, skirt, texWood, 1.f);
            // North
            m = glm::scale(glm::translate(glm::mat4(1.f), { xc,FLOOR_Y + 0.08f,z2 }), { rw,0.16f,0.06f });
            drawCube(sh, m, skirt, texWood, 1.f);
            // West
            m = glm::scale(glm::translate(glm::mat4(1.f), { x1,FLOOR_Y + 0.08f,zc }), { 0.06f,0.16f,rd });
            drawCube(sh, m, skirt, texWood, 1.f);
            // East
            m = glm::scale(glm::translate(glm::mat4(1.f), { x2,FLOOR_Y + 0.08f,zc }), { 0.06f,0.16f,rd });
            drawCube(sh, m, skirt, texWood, 1.f);

            // ── Ceiling lamp fixture ───────────────────────────────────────
            // Mounting disc
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H - 0.01f, zc }),
                { 0.55f, 0.06f, 0.55f });
            drawCylinder(sh, m, glm::vec3(0.75f, 0.73f, 0.68f));

            // Pendant cord
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H - 0.30f, zc }),
                { 0.04f, 0.50f, 0.04f });
            drawCylinder(sh, m, glm::vec3(0.20f, 0.20f, 0.22f));

            // Lamp shade (inverted cone shape → cone flipped)
            {
                glm::mat4 shade = glm::translate(glm::mat4(1.f),
                    { xc, FLOOR_Y + CLG_H - 0.55f, zc });
                shade = glm::rotate(shade, glm::radians(180.f), { 1.f,0.f,0.f });
                shade = glm::scale(shade, { 0.55f, 0.40f, 0.55f });
                drawCone(sh, shade, glm::vec3(0.78f, 0.72f, 0.60f));
            }

            // Emissive light bulb (tiny sphere inside shade)
            {
                glm::vec3 bulb(1.00f, 0.96f, 0.82f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bulb));
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { xc, FLOOR_Y + CLG_H - 0.50f, zc }),
                    { 0.18f, 0.18f, 0.18f });
                drawSphere(sh, m, bulb, texMarble, 1.f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
            }

            // ── Furniture: mirror X position for room 2 ───────────────────
            float mx = mirrorFurniture ? -1.f : 1.f;   // flip sign for room 2
            float bx = x1 + mx * 1.5f;                  // bed X anchor near west/east wall
            float tx = x2 - mx * 2.2f;                  // table X anchor near east/west wall

            // ══ BED ══════════════════════════════════════════════════════════
            float bedZ = z1 + 3.0f;   // bed centre Z (near south wall)
            // Bed frame
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 0.22f, bedZ }),
                { 2.20f, 0.44f, 4.20f });
            drawCube(sh, m, BED_FRAME, texWood, 2.f);
            // Legs (4 corners)
            for (int lz : {-1, 1}) for (int lx2 : {-1, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { bx + mx * 1.0f + lx2 * 0.9f, FLOOR_Y + 0.11f, bedZ + lz * 1.8f }),
                    { 0.18f, 0.22f, 0.18f });
                drawCylinder(sh, m, BED_FRAME);
            }
            // Mattress
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 0.52f, bedZ }),
                { 2.00f, 0.24f, 3.90f });
            drawCube(sh, m, MATTRESS, texConcrete, 1.f);
            // Blanket (folded at foot half)
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 0.66f, bedZ - 0.80f }),
                { 2.00f, 0.16f, 2.20f });
            drawCube(sh, m, BLANKET);
            // Pillow
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 0.68f, bedZ + 1.55f }),
                { 1.80f, 0.18f, 0.90f });
            drawCube(sh, m, PILLOW);
            // Headboard
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 1.10f, z1 + 0.40f }),
                { 2.20f, 1.10f, 0.16f });
            drawCube(sh, m, BED_FRAME, texWood, 1.f);
            // Headboard panel detail (inset rectangle)
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 1.10f, z1 + 0.33f }),
                { 1.80f, 0.80f, 0.06f });
            drawCube(sh, m, glm::vec3(0.48f, 0.34f, 0.18f));
            // Foot-board
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 1.0f, FLOOR_Y + 0.75f, bedZ - 2.15f }),
                { 2.20f, 0.60f, 0.16f });
            drawCube(sh, m, BED_FRAME, texWood, 1.f);
            // Small bedside table
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { bx + mx * 2.35f, FLOOR_Y + 0.35f, bedZ + 1.0f }),
                { 0.70f, 0.70f, 0.70f });
            drawCube(sh, m, TABLE_COL, texWood, 1.f);
            // Bedside lamp (small cone shade)
            {
                glm::mat4 bs = glm::translate(glm::mat4(1.f),
                    { bx + mx * 2.35f, FLOOR_Y + 0.95f, bedZ + 1.0f });
                bs = glm::rotate(bs, glm::radians(180.f), { 1.f,0.f,0.f });
                bs = glm::scale(bs, { 0.22f,0.28f,0.22f });
                drawCone(sh, bs, glm::vec3(0.82f, 0.72f, 0.45f));
            }
            {
                glm::vec3 bsb(0.98f, 0.90f, 0.65f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bsb * 0.6f));
                m = glm::scale(glm::translate(glm::mat4(1.f), { bx + mx * 2.35f,FLOOR_Y + 0.82f,bedZ + 1.0f }),
                    { 0.10f,0.10f,0.10f });
                drawSphere(sh, m, bsb, texMarble, 1.f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
            }

            // ══ STUDY TABLE + CHAIR ══════════════════════════════════════════
            float tabZ = z2 - 2.8f;
            // Table top
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { tx, FLOOR_Y + 0.75f, tabZ }),
                { 1.80f, 0.08f, 0.90f });
            drawCube(sh, m, TABLE_COL, texWood, 1.f);
            // Table legs (4)
            for (int lz : {-1, 1}) for (int lx2 : {-1, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { tx + lx2 * 0.78f, FLOOR_Y + 0.37f, tabZ + lz * 0.35f }),
                    { 0.10f, 0.74f, 0.10f });
                drawCylinder(sh, m, TABLE_COL);
            }
            // Table drawer block
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { tx, FLOOR_Y + 0.38f, tabZ - 0.28f }),
                { 1.60f, 0.62f, 0.30f });
            drawCube(sh, m, TABLE_COL, texWood, 1.f);
            // Drawer handles (2 small cubes)
            for (int d : {-1, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { tx + d * 0.38f, FLOOR_Y + 0.38f, tabZ - 0.44f }),
                    { 0.22f, 0.06f, 0.04f });
                drawCube(sh, m, glm::vec3(0.65f, 0.60f, 0.45f));
            }
            // Laptop on table (thin dark rectangle)
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { tx - mx * 0.18f, FLOOR_Y + 0.80f, tabZ - 0.04f }),
                { 0.80f, 0.04f, 0.58f });
            drawCube(sh, m, glm::vec3(0.15f, 0.15f, 0.17f));
            // Laptop screen (upright, slightly tilted)
            {
                glm::mat4 ls = glm::translate(glm::mat4(1.f),
                    { tx - mx * 0.18f, FLOOR_Y + 1.12f, tabZ - 0.30f });
                ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
                ls = glm::scale(ls, { 0.78f,0.50f,0.04f });
                drawCube(sh, ls, glm::vec3(0.12f, 0.12f, 0.15f));
                // Screen glow
                glm::vec3 sc2(0.40f, 0.60f, 0.95f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(sc2 * 0.55f));
                ls = glm::translate(glm::mat4(1.f), { tx - mx * 0.18f, FLOOR_Y + 1.12f, tabZ - 0.31f });
                ls = glm::rotate(ls, glm::radians(-15.f), { 1.f,0.f,0.f });
                ls = glm::scale(ls, { 0.70f,0.43f,0.03f });
                drawCube(sh, ls, sc2);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
            }
            // Chair (seat + back + 4 legs)
            float chairZ = tabZ + 0.80f;
            // Seat
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { tx, FLOOR_Y + 0.48f, chairZ }),
                { 0.70f, 0.08f, 0.70f });
            drawCube(sh, m, CHAIR_COL, texWood, 1.f);
            // Chair legs
            for (int lz : {-1, 1}) for (int lx2 : {-1, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { tx + lx2 * 0.28f, FLOOR_Y + 0.24f, chairZ + lz * 0.28f }),
                    { 0.08f, 0.48f, 0.08f });
                drawCylinder(sh, m, CHAIR_COL);
            }
            // Chair back
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { tx, FLOOR_Y + 0.85f, chairZ - 0.34f }),
                { 0.70f, 0.75f, 0.08f });
            drawCube(sh, m, CHAIR_COL, texWood, 1.f);
            // Chair back horizontal rails
            for (int r : {0, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { tx, FLOOR_Y + 0.60f + r * 0.30f, chairZ - 0.34f }),
                    { 0.65f, 0.06f, 0.10f });
                drawCube(sh, m, glm::vec3(0.38f, 0.28f, 0.15f));
            }

            // ══ BOOKSHELF (wall-mounted, south wall) ═════════════════════════
            float shelfX = xc;
            float shelfZ = z1 + 0.18f;
            // Shelf back panel
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { shelfX, FLOOR_Y + 1.85f, shelfZ }),
                { 3.20f, 1.40f, 0.28f });
            drawCube(sh, m, SHELF_COL, texWood, 1.f);
            // Two shelf boards
            for (int sb : {0, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { shelfX, FLOOR_Y + 1.30f + sb * 0.60f, shelfZ + 0.12f }),
                    { 3.20f, 0.06f, 0.30f });
                drawCube(sh, m, SHELF_COL, texWood, 1.f);
            }
            // Books on each shelf
            float bookX = shelfX - 1.40f;
            for (int bk = 0; bk < 7; bk++) {
                float bkH = 0.28f + (bk % 3) * 0.06f;
                int   shelfRow = (bk < 4) ? 0 : 1;
                float bkY = FLOOR_Y + 1.30f + shelfRow * 0.60f + bkH * 0.5f;
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { bookX + bk * 0.38f, bkY, shelfZ + 0.14f }),
                    { 0.10f, bkH, 0.28f });
                drawCube(sh, m, BOOK_COLS[bk % 4]);
            }

            // ══ WARDROBE ═════════════════════════════════════════════════════
            float wdX = x1 + mx * 1.60f;
            float wdZ = z2 - 0.70f;
            // Body
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { wdX + mx * 0.0f, FLOOR_Y + 1.25f, wdZ }),
                { 1.50f, 2.50f, 0.72f });
            drawCube(sh, m, WARD_COL, texWood, 2.f);
            // Top overhang
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { wdX + mx * 0.0f, FLOOR_Y + 2.52f, wdZ }),
                { 1.62f, 0.10f, 0.80f });
            drawCube(sh, m, WARD_COL, texWood, 1.f);
            // Door divider line
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { wdX + mx * 0.0f, FLOOR_Y + 1.25f, wdZ - 0.37f }),
                { 0.06f, 2.40f, 0.04f });
            drawCube(sh, m, glm::vec3(0.30f, 0.22f, 0.10f));
            // Handles
            for (int h : {-1, 1}) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { wdX + h * 0.32f, FLOOR_Y + 1.30f, wdZ - 0.38f }),
                    { 0.06f, 0.24f, 0.06f });
                drawCylinder(sh, m, glm::vec3(0.75f, 0.68f, 0.45f));
            }
            // Wardrobe mirror panel (upper half, one door)
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { wdX + mx * 0.35f, FLOOR_Y + 1.70f, wdZ - 0.38f }),
                { 0.60f, 1.30f, 0.04f });
            drawCrystalGlass(sh, m);

            // ══ NOTICE BOARD (pinned on west/east wall) ═══════════════════════
            float nbX = mirrorFurniture ? x2 - 0.08f : x1 + 0.08f;
            float nbZ = zc + 0.5f;
            // Cork backing
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { nbX, FLOOR_Y + 1.55f, nbZ }),
                { 0.08f, 0.90f, 1.40f });
            drawCube(sh, m, glm::vec3(0.68f, 0.50f, 0.28f));
            // Frame
            m = glm::scale(
                glm::translate(glm::mat4(1.f), { nbX, FLOOR_Y + 1.55f, nbZ }),
                { 0.10f, 1.00f, 1.52f });
            drawCube(sh, m, SHELF_COL);
            // Pinned papers (3 coloured rectangles)
            for (int p = 0; p < 3; p++) {
                m = glm::scale(
                    glm::translate(glm::mat4(1.f),
                        { nbX, FLOOR_Y + 1.35f + p * 0.28f,
                         nbZ - 0.45f + p * 0.40f }),
                    { 0.07f, 0.22f, 0.36f });
                drawCube(sh, m, BOOK_COLS[p]);
            }
        };  // end buildRoom lambda

    // ── Build Room 1 (left, camera enters here from front entrance) ───────
    buildRoom(R1_X1, R1_X2, R1_Z1, R1_Z2, R1_XC, R1_ZC, R1_W, R1_D, false);

    // ── Build Room 2 (right, mirrored layout) ─────────────────────────────
    buildRoom(R2_X1, R2_X2, R2_Z1, R2_Z2, R2_XC, R2_ZC, R2_W, R2_D, true);

    // ── Shared corridor strip between the two rooms (z: -1 to +9.5, x: -1.5) ─
    // Narrow hallway divider wall
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { -1.5f, FLOOR_Y + CLG_H * 0.5f, R1_ZC }),
        { WALL_T, CLG_H, R1_D + WALL_T * 2.f });
    drawCube(sh, m, WALL_COL, texConcrete, 2.f);

    // ── Room number plates (emissive above each door) ─────────────────────
    glm::vec3 plateGlow(0.90f, 0.85f, 0.65f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(plateGlow * 0.55f));
    // Room 101 plate
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { R1_XC, FLOOR_Y + CLG_H - 0.35f, R1_Z2 + WALL_T * 0.55f }),
        { 0.55f, 0.22f, 0.08f });
    drawCube(sh, m, plateGlow);
    // Room 102 plate
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { R2_XC, FLOOR_Y + CLG_H - 0.35f, R2_Z2 + WALL_T * 0.55f }),
        { 0.55f, 0.22f, 0.08f });
    drawCube(sh, m, plateGlow);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}
