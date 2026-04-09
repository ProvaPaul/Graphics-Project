#include "scene_cricket.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

// ══════════════════════════════════════════════════════════════════════════════
//  renderCricketPitch
// ══════════════════════════════════════════════════════════════════════════════
void renderCricketPitch(unsigned int sh) {
    glm::mat4 m;

    // Outfield oval (green grass)
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { CR_X, CR_Y - 0.04f, CR_Z }),
        { CR_OVAL_R * 2.0f, 0.06f, CR_OVAL_R * 2.4f });
    drawCylinder(sh, m, glm::vec3(0.22f, 0.50f, 0.18f), texGrass, 6.f);

    // Infield square
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { CR_X, CR_Y - 0.02f, CR_Z }),
        { CR_OVAL_R * 1.1f, 0.06f, CR_OVAL_R * 1.3f });
    drawCube(sh, m, glm::vec3(0.28f, 0.58f, 0.22f), texGrass, 4.f);

    // Pitch strip (brown clay)
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { CR_X, CR_Y, CR_Z }),
        { CR_PITCH_W, 0.07f, CR_PITCH_L });
    drawCube(sh, m, glm::vec3(0.62f, 0.46f, 0.28f), texConcrete, 3.f);

    // ── Crease lines ─────────────────────────────────────────────────────────
    const glm::vec3 WHITE(0.96f, 0.96f, 0.96f);
    const float LH = 0.04f, LT = 0.10f, SY = CR_Y + 0.04f;

    // Batting crease
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X,SY,CR_BAT_Z }), { CR_PITCH_W + 1.2f,LH,LT });
    drawCube(sh, m, WHITE);
    // Bowling crease
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X,SY,CR_BOWL_Z }), { CR_PITCH_W + 1.2f,LH,LT });
    drawCube(sh, m, WHITE);
    // Popping crease – batting end
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X,SY,CR_BAT_Z - 0.86f }), { CR_PITCH_W + 3.0f,LH,LT });
    drawCube(sh, m, WHITE);
    // Popping crease – bowler end
    m = glm::scale(glm::translate(glm::mat4(1.f), { CR_X,SY,CR_BOWL_Z + 0.86f }), { CR_PITCH_W + 3.0f,LH,LT });
    drawCube(sh, m, WHITE);
    // Return creases
    for (int s : {-1, 1}) {
        float rx = CR_X + s * (CR_PITCH_W * 0.5f + 0.6f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,SY,CR_BOWL_Z + 1.0f }), { LT,LH,2.0f }); drawCube(sh, m, WHITE);
        m = glm::scale(glm::translate(glm::mat4(1.f), { rx,SY,CR_BAT_Z - 1.0f }), { LT,LH,2.0f }); drawCube(sh, m, WHITE);
    }

    // ── Stumps ────────────────────────────────────────────────────────────────
    const glm::vec3 STUMP_COL(0.90f, 0.82f, 0.55f);
    const float STUMP_H = 0.72f, STUMP_R = 0.035f, STUMP_SP = 0.114f;
    for (int end : {0, 1}) {
        float sz = (end == 0) ? CR_BAT_Z : CR_BOWL_Z;
        for (int s = -1; s <= 1; s++) {
            float sx = CR_X + s * STUMP_SP;
            glm::mat4 sc = glm::scale(glm::translate(glm::mat4(1.f), { sx,CR_Y + STUMP_H * 0.5f,sz }), { STUMP_R * 2.f,STUMP_H,STUMP_R * 2.f });
            drawCylinder(sh, sc, STUMP_COL);
            m = glm::scale(glm::translate(glm::mat4(1.f), { sx,CR_Y + STUMP_H + 0.04f,sz }), { 0.12f,0.05f,0.05f });
            drawCube(sh, m, glm::vec3(0.85f, 0.75f, 0.40f));
        }
    }

    // ── Full Boundary Fence (like badminton) ─────────────────────────────────---
    const glm::vec3 FENCE(0.55f, 0.38f, 0.22f);
    const float BF_H = 2.5f;
    const float BX_EXT = CR_OVAL_R + 1.5f;
    const float BZ_EXT = CR_OVAL_R * 1.2f + 1.5f;
    
    // Left and right walls - properly sized to cover the oval
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), 
            { CR_X + s * BX_EXT, BF_H * 0.5f, CR_Z }), 
            { 0.3f, BF_H, BZ_EXT * 2.f });
        drawCube(sh, m, FENCE, texWood, 2.f);
    }
    // Front and back walls - properly sized to cover the oval
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), 
            { CR_X, BF_H * 0.5f, CR_Z + s * BZ_EXT }), 
            { BX_EXT * 2.f + 0.6f, BF_H, 0.3f });
        drawCube(sh, m, FENCE, texWood, 2.f);
    }
    
    // ── Floodlight Poles around cricket ground ─────────────────────────────────
    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    for (int s : {-1, 1}) {
        for (int t : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), 
                { CR_X + s * (BX_EXT + 0.5f), 6.f, CR_Z + t * (BZ_EXT + 0.5f) }), 
                { 0.25f, 12.f, 0.25f });
            drawCylinder(sh, m, STEEL);
            // Light fixture
            m = glm::scale(glm::translate(glm::mat4(1.f), 
                { CR_X + s * (BX_EXT + 0.5f), 12.2f, CR_Z + t * (BZ_EXT + 0.5f) }), 
                { 0.7f, 0.4f, 1.2f });
            drawCube(sh, m, glm::vec3(0.88f, 0.88f, 0.20f));
            if (!dayMode) {
                static const glm::vec3 lemit(0.9f, 0.9f, 0.15f);
                glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            }
            m = glm::scale(glm::translate(glm::mat4(1.f), 
                { CR_X + s * (BX_EXT + 0.5f), 11.8f, CR_Z + t * (BZ_EXT + 0.5f) }), 
                { 0.6f, 0.06f, 1.0f });
            drawCube(sh, m, glm::vec3(1.0f, 1.0f, 0.6f));
            static const glm::vec3 ZERO(0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO));
        }
    }

    // ── Boundary rope posts ─────────────────────────────────────────────────────

    // Scoreboard/TV and spectator stands removed as requested.

    // ── Umpire (square leg) ───────────────────────────────────────────────────
    {
        float uX = CR_X + CR_PITCH_W * 0.5f + 1.8f, uZ = CR_BAT_Z - 1.0f;
        const glm::vec3 UMPIRE_SHIRT(0.96f, 0.96f, 0.94f);
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { uX + s * 0.12f,CR_Y + 0.55f,uZ }), { 0.18f,0.70f,0.20f });
            drawCube(sh, m, glm::vec3(0.20f, 0.20f, 0.22f));
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX,CR_Y + 1.35f,uZ }), { 0.46f,0.65f,0.30f });
        drawCube(sh, m, UMPIRE_SHIRT);
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX,CR_Y + 1.95f,uZ }), { 0.36f,0.36f,0.36f });
        drawSphere(sh, m, glm::vec3(0.85f, 0.68f, 0.50f), texMarble, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX,CR_Y + 2.20f,uZ }), { 0.40f,0.14f,0.40f });
        drawCylinder(sh, m, glm::vec3(0.92f, 0.92f, 0.88f));
        // Raised arm (signal)
        m = glm::translate(glm::mat4(1.f), { uX + 0.30f,CR_Y + 1.55f,uZ });
        m = glm::rotate(m, glm::radians(-65.f), { 0.f,0.f,1.f });
        m = glm::scale(m, { 0.12f,0.55f,0.12f });
        drawCube(sh, m, UMPIRE_SHIRT);
    }

    // ── On-field umpire (behind bowler) ───────────────────────────────────────
    {
        float uX2 = CR_X - 0.6f, uZ2 = CR_BOWL_Z - 2.5f;
        const glm::vec3 UMPIRE_SHIRT(0.96f, 0.96f, 0.94f);
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { uX2 + s * 0.12f,CR_Y + 0.55f,uZ2 }), { 0.18f,0.70f,0.20f });
            drawCube(sh, m, glm::vec3(0.20f, 0.20f, 0.22f));
        }
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX2,CR_Y + 1.35f,uZ2 }), { 0.46f,0.65f,0.30f });
        drawCube(sh, m, UMPIRE_SHIRT);
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX2,CR_Y + 1.95f,uZ2 }), { 0.36f,0.36f,0.36f });
        drawSphere(sh, m, glm::vec3(0.85f, 0.68f, 0.50f), texMarble, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { uX2,CR_Y + 2.20f,uZ2 }), { 0.40f,0.14f,0.40f });
        drawCylinder(sh, m, glm::vec3(0.92f, 0.92f, 0.88f));
        // Arms at side
        for (int s : {-1, 1}) {
            m = glm::translate(glm::mat4(1.f), { uX2 + s * 0.32f, CR_Y + 1.55f, uZ2 });
            m = glm::rotate(m, glm::radians(s * -20.f), { 0.f,0.f,1.f });
            m = glm::scale(m, { 0.12f,0.50f,0.12f });
            drawCube(sh, m, UMPIRE_SHIRT);
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  Helper: draw a single fielder/player body at world position
// ══════════════════════════════════════════════════════════════════════════════
static void drawPlayerBody(unsigned int sh, float px, float py, float pz,
    glm::vec3 shirtCol, glm::vec3 skinCol,
    bool hasCap = true, glm::vec3 capCol = { 0.15f,0.22f,0.55f })
{
    glm::mat4 m;
    const glm::vec3 PANTS(0.96f, 0.95f, 0.92f);

    // Legs
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { px + s * 0.12f, py + 0.60f, pz }), { 0.18f,0.75f,0.18f });
        drawCube(sh, m, PANTS);
    }
    // Torso
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, py + 1.38f, pz }), { 0.44f,0.62f,0.28f });
    drawCube(sh, m, shirtCol);
    // Head
    m = glm::scale(glm::translate(glm::mat4(1.f), { px, py + 1.95f, pz }), { 0.36f,0.36f,0.36f });
    drawSphere(sh, m, skinCol, texMarble, 1.f);
    if (hasCap) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { px, py + 2.18f, pz }), { 0.38f,0.12f,0.38f });
        drawCylinder(sh, m, capCol);
    }
    // Arms
    for (int s : {-1, 1}) {
        m = glm::translate(glm::mat4(1.f), { px + s * 0.30f, py + 1.55f, pz });
        m = glm::rotate(m, glm::radians(s * -15.f), { 0.f,0.f,1.f });
        m = glm::scale(m, { 0.13f,0.46f,0.13f });
        drawCube(sh, m, shirtCol);
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  renderBatsman
//  FIX: bat downswing is now timed so it connects with the ball at phase ~0.52
//       (ball arrives at CR_BAT_Z at phase 0.52).
//  Added: non-striker at bowler's end + wicket-keeper behind stumps.
// ══════════════════════════════════════════════════════════════════════════════
void renderBatsman(unsigned int sh) {
    float t = (float)glfwGetTime();
    float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;

    // ─────────────────────────────────────────────────────────────────────────
    //  Bat swing phases – FIXED timing so contact happens at phase ≈ 0.52
    //
    //  Phase 0.00–0.35 : Ready stance         batAngle = -15°
    //  Phase 0.35–0.50 : Backswing            batAngle  -15° → +48°
    //  Phase 0.50–0.65 : Downswing / CONTACT  batAngle  +48° → -52°  ← ball arrives 0.52
    //  Phase 0.65–1.00 : Follow-through reset  batAngle  -52° → -15°
    // ─────────────────────────────────────────────────────────────────────────
    float batAngle = 0.f, leanZ = 0.f;
    if (phase < 0.35f) {
        batAngle = -15.f;
        leanZ = 0.f;
    }
    else if (phase < 0.50f) {
        float p = (phase - 0.35f) / 0.15f;
        batAngle = -15.f + p * 63.f;           // -15 → +48
        leanZ = p * 0.20f;
    }
    else if (phase < 0.65f) {
        float p = (phase - 0.50f) / 0.15f;
        batAngle = 48.f - p * 100.f;          // +48 → -52  (contact mid-swing)
        leanZ = 0.20f - p * 0.12f;
    }
    else {
        float p = (phase - 0.65f) / 0.35f;
        batAngle = -52.f + p * 37.f;           // -52 → -15
        leanZ = 0.08f - p * 0.08f;
    }

    const glm::vec3 WHITES(0.96f, 0.95f, 0.92f);
    const glm::vec3 PAD_COL(0.94f, 0.93f, 0.88f);
    const glm::vec3 HELMET_COL(0.15f, 0.22f, 0.55f);
    const glm::vec3 GLOVE_COL(0.90f, 0.88f, 0.82f);
    const glm::vec3 BAT_HANDLE(0.35f, 0.24f, 0.12f);
    const glm::vec3 BAT_BLADE(0.80f, 0.65f, 0.35f);
    const glm::vec3 SHOE_COL(0.12f, 0.10f, 0.10f);
    const glm::vec3 SKIN_COL(0.82f, 0.62f, 0.44f);

    float bX = CR_X - 0.25f, bZ = CR_BAT_Z - 0.40f + leanZ, bY = CR_Y;
    glm::mat4 m;

    // ── Shoes ──────────────────────────────────────────────────────────────
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f),
            { bX + s * 0.16f, bY + 0.12f, bZ + (s == -1 ? leanZ * 1.5f : 0.f) }),
            { 0.20f,0.22f,0.40f });
        drawCube(sh, m, SHOE_COL);
    }

    // ── Batting pads ───────────────────────────────────────────────────────
    for (int s : {-1, 1}) {
        m = glm::scale(glm::translate(glm::mat4(1.f), { bX + s * 0.16f, bY + 0.72f, bZ }),
            { 0.26f,0.90f,0.32f });
        drawCube(sh, m, PAD_COL);
        for (int bk = 0; bk < 3; bk++) {
            m = glm::scale(glm::translate(glm::mat4(1.f),
                { bX + s * 0.16f, bY + 0.35f + bk * 0.28f, bZ + 0.17f }),
                { 0.28f,0.05f,0.04f });
            drawCube(sh, m, glm::vec3(0.60f, 0.55f, 0.30f));
        }
    }

    // ── Shorts / thigh pad ─────────────────────────────────────────────────
    m = glm::scale(glm::translate(glm::mat4(1.f), { bX, bY + 1.18f, bZ }), { 0.50f,0.26f,0.34f });
    drawCube(sh, m, WHITES);

    // ── Torso ──────────────────────────────────────────────────────────────
    m = glm::scale(glm::translate(glm::mat4(1.f), { bX, bY + 1.65f, bZ }), { 0.52f,0.70f,0.34f });
    drawCube(sh, m, WHITES);

    // ── Helmet dome ────────────────────────────────────────────────────────
    m = glm::scale(glm::translate(glm::mat4(1.f), { bX, bY + 2.28f, bZ }), { 0.46f,0.38f,0.46f });
    drawSphere(sh, m, HELMET_COL, texConcrete, 1.f);
    // Visor
    m = glm::scale(glm::translate(glm::mat4(1.f), { bX, bY + 2.12f, bZ + 0.24f }), { 0.42f,0.10f,0.12f });
    drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.10f));
    // Grill
    m = glm::scale(glm::translate(glm::mat4(1.f), { bX, bY + 2.05f, bZ + 0.28f }), { 0.36f,0.22f,0.06f });
    drawCube(sh, m, glm::vec3(0.35f, 0.35f, 0.38f));

    // ── Left arm (guide hand) ──────────────────────────────────────────────
    {
        glm::mat4 la = glm::translate(glm::mat4(1.f), { bX + 0.32f, bY + 1.70f, bZ + 0.10f });
        la = glm::rotate(la, glm::radians(-20.f + batAngle * 0.3f), { 1.f,0.f,0.f });
        m = glm::scale(la, { 0.14f,0.50f,0.14f }); drawCube(sh, m, WHITES);

        glm::mat4 lfa = glm::translate(la, { 0.f,0.50f,0.f });
        lfa = glm::rotate(lfa, glm::radians(-10.f), { 1.f,0.f,0.f });
        m = glm::scale(lfa, { 0.13f,0.42f,0.13f }); drawCube(sh, m, WHITES);
        m = glm::scale(glm::translate(lfa, { 0.f,0.25f,0.f }), { 0.20f,0.22f,0.20f });
        drawCube(sh, m, GLOVE_COL);
    }

    // ── Right arm + bat ────────────────────────────────────────────────────
    {
        glm::mat4 ra = glm::translate(glm::mat4(1.f), { bX - 0.32f, bY + 1.72f, bZ - 0.05f });
        ra = glm::rotate(ra, glm::radians(batAngle * 0.8f), { 1.f,0.f,0.f });
        ra = glm::rotate(ra, glm::radians(12.f), { 0.f,0.f,1.f });
        m = glm::scale(ra, { 0.14f,0.50f,0.14f }); drawCube(sh, m, WHITES);

        glm::mat4 rfa = glm::translate(ra, { 0.f,0.50f,0.f });
        rfa = glm::rotate(rfa, glm::radians(batAngle * 0.4f), { 1.f,0.f,0.f });
        m = glm::scale(rfa, { 0.13f,0.42f,0.13f }); drawCube(sh, m, WHITES);

        glm::mat4 glv = glm::translate(rfa, { 0.f,0.26f,0.f });
        m = glm::scale(glv, { 0.20f,0.22f,0.20f }); drawCube(sh, m, GLOVE_COL);

        // Bat handle
        glm::mat4 batRoot = glm::translate(glv, { 0.f,0.22f,0.f });
        batRoot = glm::rotate(batRoot, glm::radians(batAngle * 0.6f), { 1.f,0.f,0.f });
        m = glm::scale(batRoot, { 0.08f,0.52f,0.08f });
        drawCylinder(sh, m, BAT_HANDLE, texWood, 1.f);

        // Bat blade
        glm::mat4 blade = glm::translate(batRoot, { 0.f,0.50f,0.f });
        blade = glm::rotate(blade, glm::radians(5.f), { 1.f,0.f,0.f });
        m = glm::scale(blade, { 0.18f,0.72f,0.06f }); drawCube(sh, m, BAT_BLADE, texWood, 1.f);
        // Edge ridge
        m = glm::scale(glm::translate(blade, { 0.10f,0.f,0.f }), { 0.04f,0.72f,0.06f });
        drawCube(sh, m, glm::vec3(0.55f, 0.40f, 0.18f));
    }

    // ══════════════════════════════════════════════════════════════════════
    //  BOWLER
    // ══════════════════════════════════════════════════════════════════════
    const glm::vec3 BOWL_SHIRT(0.85f, 0.20f, 0.25f);
    const glm::vec3 BOWL_PANT(0.96f, 0.95f, 0.92f);
    const glm::vec3 BOWL_SKIN(0.82f, 0.62f, 0.44f);

    float wX = CR_X + 0.20f, wZ = CR_BOWL_Z + 0.60f;

    // Bowling arm: release ball at phase=0 (start of delivery), match ball travel
    float strideAngle = -18.f + 18.f * sinf(t * (2.f * PI / CR_CYCLE));
    float bowlArmAngle = 220.f + 80.f * sinf(t * (2.f * PI / CR_CYCLE) + 0.8f);

    // Back leg
    m = glm::scale(glm::translate(glm::mat4(1.f), { wX - 0.14f, bY + 0.70f, wZ + 0.15f }),
        { 0.22f,0.90f,0.22f });
    drawCube(sh, m, BOWL_PANT);

    // Front leg (striding)
    {
        glm::mat4 fl = glm::translate(glm::mat4(1.f), { wX + 0.14f, bY + 0.70f, wZ - 0.35f });
        fl = glm::rotate(fl, glm::radians(strideAngle), { 1.f,0.f,0.f });
        m = glm::scale(fl, { 0.22f,0.90f,0.22f }); drawCube(sh, m, BOWL_PANT);
    }

    // Torso
    m = glm::scale(glm::translate(glm::mat4(1.f), { wX, bY + 1.55f, wZ }), { 0.50f,0.68f,0.32f });
    drawCube(sh, m, BOWL_SHIRT, texConcrete, 1.f);

    // Head
    m = glm::scale(glm::translate(glm::mat4(1.f), { wX, bY + 2.18f, wZ }), { 0.40f,0.40f,0.40f });
    drawSphere(sh, m, BOWL_SKIN, texMarble, 1.f);

    // Cap
    m = glm::scale(glm::translate(glm::mat4(1.f), { wX, bY + 2.42f, wZ }), { 0.42f,0.14f,0.42f });
    drawCylinder(sh, m, glm::vec3(0.85f, 0.20f, 0.25f)); // red cap = bowling team

    // Non-bowling arm
    {
        glm::mat4 na = glm::translate(glm::mat4(1.f), { wX + 0.30f, bY + 1.70f, wZ });
        na = glm::rotate(na, glm::radians(-50.f), { 0.f,0.f,1.f });
        m = glm::scale(na, { 0.13f,0.46f,0.13f }); drawCube(sh, m, BOWL_SHIRT);
    }

    // Bowling arm (animated – full windmill)
    {
        glm::mat4 ba = glm::translate(glm::mat4(1.f), { wX - 0.30f, bY + 1.68f, wZ });
        ba = glm::rotate(ba, glm::radians(bowlArmAngle), { 1.f,0.f,0.f });
        ba = glm::rotate(ba, glm::radians(20.f), { 0.f,0.f,1.f });
        m = glm::scale(ba, { 0.13f,0.50f,0.13f }); drawCube(sh, m, BOWL_SHIRT);

        glm::mat4 bfa = glm::translate(ba, { 0.f,0.50f,0.f });
        bfa = glm::rotate(bfa, glm::radians(20.f), { 1.f,0.f,0.f });
        m = glm::scale(bfa, { 0.12f,0.42f,0.12f }); drawCube(sh, m, BOWL_SKIN);
    }

    // ══════════════════════════════════════════════════════════════════════
    //  NON-STRIKER BATSMAN (standing at bowler's end crease)
    //  Stands just behind CR_BOWL_Z, holds bat upright, faces batting end
    // ══════════════════════════════════════════════════════════════════════
    {
        float nsX = CR_X + 0.55f;     // slightly off-pitch to bowler's left
        float nsZ = CR_BOWL_Z - 0.50f;

        // Shoes
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { nsX + s * 0.14f, bY + 0.12f, nsZ }),
                { 0.18f,0.20f,0.36f });
            drawCube(sh, m, SHOE_COL);
        }
        // Pads (lighter – not facing)
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { nsX + s * 0.14f, bY + 0.68f, nsZ }),
                { 0.22f,0.80f,0.28f });
            drawCube(sh, m, PAD_COL);
        }
        // Body
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX, bY + 1.18f, nsZ }), { 0.44f,0.24f,0.30f });
        drawCube(sh, m, WHITES);
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX, bY + 1.60f, nsZ }), { 0.48f,0.66f,0.30f });
        drawCube(sh, m, WHITES);
        // Helmet
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX, bY + 2.24f, nsZ }), { 0.44f,0.36f,0.44f });
        drawSphere(sh, m, HELMET_COL, texConcrete, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX, bY + 2.10f, nsZ - 0.22f }), { 0.40f,0.10f,0.12f });
        drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.10f));
        // Gloves + upright bat (resting on ground)
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX - 0.28f, bY + 1.60f, nsZ }), { 0.13f,0.46f,0.13f });
        drawCube(sh, m, WHITES);
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX - 0.28f, bY + 1.05f, nsZ }), { 0.18f,0.20f,0.18f });
        drawCube(sh, m, GLOVE_COL);
        // Bat handle (upright, held at side)
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX - 0.32f, bY + 0.70f, nsZ }), { 0.07f,0.80f,0.07f });
        drawCylinder(sh, m, BAT_HANDLE, texWood, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { nsX - 0.32f, bY + 0.34f, nsZ }), { 0.17f,0.65f,0.05f });
        drawCube(sh, m, BAT_BLADE, texWood, 1.f);
    }

    // ══════════════════════════════════════════════════════════════════════
    //  WICKET-KEEPER – crouched just behind batting stumps
    //  Gloves rise to catch as ball arrives (phase 0.52–0.65 only if missed)
    // ══════════════════════════════════════════════════════════════════════
    {
        const glm::vec3 WK_SHIRT(0.85f, 0.20f, 0.25f);
        const glm::vec3 WK_PAD(0.94f, 0.93f, 0.88f);
        const glm::vec3 WK_GLOVE(0.96f, 0.92f, 0.55f); // yellow gloves

        float kX = CR_X + 0.30f;                // slightly off-stump
        float kZ = CR_BAT_Z + 1.20f;            // behind batting stumps
        float kY = bY;

        // Crouch fraction: keeper squats as ball approaches, rises on missed ball
        // During hit (phase 0.50-0.75) keeper stands up to watch the shot
        float crouchFrac = 0.0f;
        if (phase < 0.40f) {
            crouchFrac = 0.35f;                  // slightly crouched, ready
        }
        else if (phase < 0.52f) {
            float p = (phase - 0.40f) / 0.12f;
            crouchFrac = 0.35f + p * 0.30f;     // deeper crouch as ball nears
        }
        else if (phase < 0.68f) {
            float p = (phase - 0.52f) / 0.16f;
            crouchFrac = 0.65f - p * 0.65f;     // rise up after ball hit
        }

        float legH = 0.75f - crouchFrac * 0.30f;
        float bodyY = kY + legH * 0.5f + 0.10f;
        float torsoH = 0.60f - crouchFrac * 0.18f;
        float headY = bodyY + torsoH * 0.5f + 0.20f;

        // Shoes
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { kX + s * 0.16f, kY + 0.10f, kZ }),
                { 0.18f,0.18f,0.34f });
            drawCube(sh, m, SHOE_COL);
        }
        // Pads (keeper wears full pads)
        for (int s : {-1, 1}) {
            m = glm::scale(glm::translate(glm::mat4(1.f), { kX + s * 0.16f, kY + 0.52f, kZ }),
                { 0.24f, legH * 0.85f, 0.28f });
            drawCube(sh, m, WK_PAD);
        }
        // Torso
        m = glm::scale(glm::translate(glm::mat4(1.f), { kX, bodyY, kZ }), { 0.48f, torsoH, 0.30f });
        drawCube(sh, m, WK_SHIRT);
        // Head + helmet
        m = glm::scale(glm::translate(glm::mat4(1.f), { kX, headY, kZ }), { 0.40f,0.36f,0.40f });
        drawSphere(sh, m, HELMET_COL, texConcrete, 1.f);
        m = glm::scale(glm::translate(glm::mat4(1.f), { kX, headY + 0.22f, kZ }), { 0.42f,0.12f,0.42f });
        drawCylinder(sh, m, HELMET_COL);

        // Keeper gloves – animate upward as ball arrives at bat
        float gloveRise = 0.0f;
        if (phase >= 0.42f && phase < 0.58f) {
            float gp = (phase - 0.42f) / 0.16f;
            gloveRise = sinf(gp * PI) * 0.28f;  // smooth rise and fall
        }
        for (int s : {-1, 1}) {
            glm::mat4 ga = glm::translate(glm::mat4(1.f),
                { kX + s * 0.32f, bodyY + 0.05f + gloveRise, kZ - 0.15f });
            ga = glm::rotate(ga, glm::radians(s * -30.f), { 0.f,0.f,1.f });
            // Upper arm
            m = glm::scale(ga, { 0.13f, 0.44f, 0.13f }); drawCube(sh, m, WK_SHIRT);
            // Forearm
            glm::mat4 fa = glm::translate(ga, { 0.f, 0.44f, 0.f });
            fa = glm::rotate(fa, glm::radians(s * 20.f), { 0.f,0.f,1.f });
            m = glm::scale(fa, { 0.12f, 0.38f, 0.12f }); drawCube(sh, m, WK_SHIRT);
            // Glove
            m = glm::scale(glm::translate(fa, { 0.f, 0.24f, 0.f }), { 0.22f, 0.22f, 0.22f });
            drawCube(sh, m, WK_GLOVE);
        }
    }

    // ══════════════════════════════════════════════════════════════════════
    //  FIELDERS (4 in outfield, animated slight body sway)
    // ══════════════════════════════════════════════════════════════════════
    const glm::vec3 FIELD_SHIRT(0.15f, 0.22f, 0.55f);
    const glm::vec3 FIELD_SKIN(0.82f, 0.62f, 0.44f);

    struct FielderPos { float x, z; };
    FielderPos fielders[] = {
        { CR_X - 10.f, CR_Z + 8.f  },  // mid-on region
        { CR_X + 10.f, CR_Z + 8.f  },  // mid-off region
        { CR_X - 12.f, CR_Z - 6.f  },  // fine leg
        { CR_X + 12.f, CR_Z - 6.f  },  // point / cover
    };
    for (auto& f : fielders) {
        drawPlayerBody(sh, f.x, bY, f.z, FIELD_SHIRT, FIELD_SKIN, true, FIELD_SHIRT);
    }

    // ── 3 more close-in fielders (slip cordon) ────────────────────────────
    float slipX[] = { CR_X + 1.8f, CR_X + 2.8f, CR_X + 3.8f };
    float slipZ = CR_BAT_Z + 2.0f;
    for (float sx : slipX) {
        drawPlayerBody(sh, sx, bY, slipZ, FIELD_SHIRT, FIELD_SKIN, true, FIELD_SHIRT);
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  renderCricketBall
//  FIX: ball arrives at CR_BAT_Z at phase ≈ 0.52, matching the bat downswing.
//       After contact (phase 0.52) the ball flies off to the boundary.
// ══════════════════════════════════════════════════════════════════════════════
void renderCricketBall(unsigned int sh) {
    float t = (float)glfwGetTime();
    float phase = fmodf(t, CR_CYCLE) / CR_CYCLE;

    const glm::vec3 RED_LEATHER(0.72f, 0.08f, 0.06f);
    const glm::vec3 SEAM_WHITE(0.92f, 0.90f, 0.86f);
    const float BALL_R = 0.13f;
    float       spinAngle = t * 720.f;

    float ballX, ballY, ballZ;

    // ─────────────────────────────────────────────────────────────────────
    //  Phase 0.00–0.52 : Delivery – bowl from CR_BOWL_Z, bounce, reach bat
    //  Phase 0.52–0.68 : Contact  – ball deflects upward/sideways off bat
    //  Phase 0.68–1.00 : Shot     – ball flies to the boundary
    // ─────────────────────────────────────────────────────────────────────
    if (phase < 0.52f) {
        // ----- DELIVERY -----
        float p = phase / 0.52f;                              // 0→1 over delivery
        ballZ = CR_BOWL_Z + (CR_BAT_Z - CR_BOWL_Z) * p;   // bowl→bat along pitch
        ballX = CR_X + 0.05f * sinf(p * PI * 2.f);         // slight lateral swing

        // Bounce at 60% of the way (good-length ball)
        const float BOUNCE_P = 0.60f;
        if (p < BOUNCE_P) {
            float q = p / BOUNCE_P;
            float yFall = 1.80f - 1.80f * q * q;                // parabolic fall
            float seam = 0.04f * fabsf(sinf(p * PI * 6.f));    // seam wobble
            ballY = CR_Y + yFall + seam;
        }
        else {
            // After pitch: rises toward batsman's hitting zone
            float q = (p - BOUNCE_P) / (1.f - BOUNCE_P);
            ballY = CR_Y + 0.75f * q;                         // rises to ~0.75 at bat
        }
    }
    else if (phase < 0.68f) {
        // ----- CONTACT – ball briefly at bat, slight upward kick -----
        float p = (phase - 0.52f) / 0.16f;
        ballZ = CR_BAT_Z - 0.04f + 0.04f * p;
        ballX = CR_X - 0.22f * p;                           // moves off-side
        ballY = CR_Y + 0.75f + 0.30f * sinf(p * PI);       // upward arc off bat
    }
    else {
        // ----- SHOT – drives to boundary -----
        float p = (phase - 0.68f) / 0.32f;
        ballZ = CR_BAT_Z + p * 12.f;                       // racing to boundary
        ballX = CR_X - 0.22f - p * 9.f;                     // angled drive
        // Ground bounce after drive: small bounces along grass
        ballY = CR_Y + 0.16f + 0.16f * fabsf(sinf(p * PI * 5.f));
    }

    glm::mat4 base = glm::translate(glm::mat4(1.f), { ballX, ballY, ballZ });
    base = glm::rotate(base, glm::radians(spinAngle), { 1.f,0.f,0.f });

    // ── Ball body ──────────────────────────────────────────────────────────
    glm::mat4 m = glm::scale(base, { BALL_R * 2.f, BALL_R * 2.f, BALL_R * 2.f });
    drawSphere(sh, m, RED_LEATHER, texMarble, 1.f);

    // ── Seam ring ──────────────────────────────────────────────────────────
    glm::mat4 seamMat = glm::rotate(base, glm::radians(90.f), { 0.f,0.f,1.f });
    seamMat = glm::scale(seamMat, { BALL_R * 1.92f, 0.025f, BALL_R * 1.92f });
    drawCylinder(sh, seamMat, SEAM_WHITE);

    // ── Ground shadow (grows closer to ball as it lands) ───────────────────
    float heightAbove = ballY - CR_Y;
    float shadowSc = 0.18f + 0.12f * (1.f - std::min(heightAbove / 2.0f, 1.f));
    glm::mat4 shadow = glm::scale(
        glm::translate(glm::mat4(1.f), { ballX, CR_Y + 0.005f, ballZ }),
        { shadowSc * 2.2f, 0.015f, shadowSc * 2.2f });
    drawCylinder(sh, shadow, glm::vec3(0.04f, 0.09f, 0.04f));
}