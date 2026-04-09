#include "scene.h"
#include "globals.h"
#include "draw_helpers.h"
#include "scene_ground.h"
#include "scene_buildings.h"
#include "scene_props.h"
#include "scene_cricket.h"
#include "curves.h"
#include "room.h"          // renderRoom(), inRoomView
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

// ── Badminton layout constants ──────────────────────────────────────────────
static const float BC_X = -62.f;
static const float BC_Z = 8.f;
static const float BC_Y = 0.06f;
static const float BC_LEN = 24.00f;
static const float BC_WID = 12.00f;

// ════════════════════════════════════════════════════════════════════════════
//  renderBadmintonCourt  (unchanged)
// ════════════════════════════════════════════════════════════════════════════
static void renderBadmintonCourt(unsigned int sh) {
    glm::mat4 m;

    // Court surface (green synthetic mat)
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { BC_X, BC_Y - 0.03f, BC_Z }),
        { BC_WID + 2.f, 0.08f, BC_LEN + 2.f });
    drawCube(sh, m, glm::vec3(0.18f, 0.45f, 0.18f), texGrass, 3.f);

    const float LT = 0.12f, LH = 0.04f, SY = BC_Y + 0.02f;
    const glm::vec3 WHITE(0.96f, 0.96f, 0.96f);

    // Side lines
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { BC_X + s * BC_WID * 0.5f, SY, BC_Z }),
            { LT, LH, BC_LEN });
        drawCube(sh, m, WHITE);
    }
    // Back lines
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * BC_LEN * 0.5f }),
            { BC_WID, LH, LT });
        drawCube(sh, m, WHITE);
    }
    // Centre line
    m = glm::scale(glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z }), { LT, LH, BC_LEN });
    drawCube(sh, m, WHITE);
    // Short service lines
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { BC_X, SY, BC_Z + s * 1.98f }),
            { BC_WID, LH, LT });
        drawCube(sh, m, WHITE);
    }
    // Long service lines
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X + s * (BC_WID * 0.5f - 0.76f), SY, BC_Z }),
            { LT, LH, BC_LEN });
        drawCube(sh, m, WHITE);
    }

    // Net posts
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X + s * (BC_WID * 0.5f + 0.2f), 0.77f, BC_Z }),
            { 0.1f, 1.55f, 0.1f });
        drawCylinder(sh, m, glm::vec3(0.85f, 0.82f, 0.75f));
    }
    // Net body
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { BC_X, 0.85f, BC_Z }),
        { BC_WID + 0.4f, 0.6f, 0.08f });
    drawCube(sh, m, glm::vec3(0.92f, 0.92f, 0.92f));
    // Net top tape
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { BC_X, 1.18f, BC_Z }),
        { BC_WID + 0.4f, 0.08f, 0.10f });
    drawCube(sh, m, WHITE);

    // Perimeter fence - properly sized to cover the court
    const glm::vec3 FENCE(0.55f, 0.38f, 0.22f);
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X + s * (BC_WID * 0.5f + 1.0f), 0.5f, BC_Z }),
            { 0.2f, 1.0f, BC_LEN + 2.0f });
        drawCube(sh, m, FENCE, texWood, 2.f);
    }
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X, 0.5f, BC_Z + s * (BC_LEN * 0.5f + 1.0f) }),
            { BC_WID + 2.0f, 1.0f, 0.2f });
        drawCube(sh, m, FENCE, texWood, 2.f);
    }

    // Floodlight poles
    const glm::vec3 STEEL(0.50f, 0.50f, 0.52f);
    for (int s : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X + s * (BC_WID * 0.5f + 1.5f), 5.f, BC_Z }),
            { 0.18f, 10.f, 0.18f });
        drawCylinder(sh, m, STEEL);
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { BC_X + s * (BC_WID * 0.5f + 1.5f), 10.1f, BC_Z }),
            { 0.5f, 0.3f, 0.8f });
        drawCube(sh, m, glm::vec3(0.88f, 0.88f, 0.20f));
        if (!dayMode) {
            static const glm::vec3 lemit(0.85f, 0.85f, 0.10f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(lemit));
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { BC_X + s * (BC_WID * 0.5f + 1.5f), 9.7f, BC_Z }),
                { 1.2f, 1.2f, 1.2f });
            drawCone(sh, m, glm::vec3(0.95f, 0.90f, 0.50f));
            static const glm::vec3 zero2(0.f, 0.f, 0.f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero2));
        }
    }
    // Cable across top
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { BC_X, 10.f, BC_Z }),
        { BC_WID + 3.2f, 0.15f, 0.15f });
    drawCube(sh, m, STEEL);
}

// ════════════════════════════════════════════════════════════════════════════
//  renderBadmintonPlayers
//
//  FIXES vs original:
//   1. Swing PEAKS at the moment the shuttle leaves the player (phase≈0 for
//      P1, phase≈0.5 for P2), not in the middle of its flight.
//   2. Both the racket arm AND the free arm are drawn for each player.
//   3. Racket-arm X offset is mirrored for player 2 (who faces south).
//   4. Body lean follows swing direction.
//
//  Phase convention (matches renderShuttle):
//   0.00–0.50 : shuttle travels south→north  (Player 1 just hit; P2 receives)
//   0.50–1.00 : shuttle travels north→south  (Player 2 just hit; P1 receives)
// ════════════════════════════════════════════════════════════════════════════
static void renderBadmintonPlayers(unsigned int sh, float phase) {

    // ── Swing-angle curve ─────────────────────────────────────────────────
    // hp = "hit-phase" where 0 = moment of contact, then 0→1 = post-hit cycle
    //   0.00–0.08 : fast downswing through contact   65° → -25°
    //   0.08–0.25 : follow-through                  -25° →  20°
    //   0.25–1.00 : wind-up for next shot            20° →  65°
    auto swingAngle = [](float hp) -> float {
        if (hp < 0.08f) {
            return 65.f - (hp / 0.08f) * 90.f;          // 65° → -25°
        }
        else if (hp < 0.25f) {
            float p = (hp - 0.08f) / 0.17f;
            return -25.f + p * 45.f;                     // -25° → 20°
        }
        else {
            float p = (hp - 0.25f) / 0.75f;
            return 20.f + p * 45.f;                      // 20° → 65°
        }
        };

    // Player 1 hits at phase ≈ 0 → hp1 = phase (0 right after hit)
    // Player 2 hits at phase ≈ 0.5 → hp2 = (phase + 0.5) mod 1 (0 right after hit)
    float hp1 = phase;
    float hp2 = fmodf(phase + 0.5f, 1.0f);

    float arm1Angle = swingAngle(hp1);
    float arm2Angle = swingAngle(hp2);

    // Lateral foot-work sway (player moves left/right while ready)
    float sway1 = 0.28f * sinf(hp1 * PI * 2.f);
    float sway2 = 0.28f * sinf(hp2 * PI * 2.f);

    // Body lean toward net during swing, back to neutral during wind-up
    float lean1 = (arm1Angle > 30.f) ? 0.f : 0.12f;
    float lean2 = (arm2Angle > 30.f) ? 0.f : 0.12f;

    // ── Colors ────────────────────────────────────────────────────────────
    const glm::vec3 SHIRT1(0.20f, 0.40f, 0.85f);
    const glm::vec3 SHIRT2(0.85f, 0.22f, 0.20f);
    const glm::vec3 SHORTS(0.15f, 0.15f, 0.15f);
    const glm::vec3 SKIN(0.88f, 0.70f, 0.55f);
    const glm::vec3 SHOE(0.15f, 0.12f, 0.10f);
    const glm::vec3 RACKET1(0.18f, 0.55f, 0.25f);
    const glm::vec3 RACKET2(0.80f, 0.55f, 0.12f);
    const glm::vec3 HANDLE_COL(0.40f, 0.28f, 0.12f);
    const glm::vec3 STRING_COL(0.92f, 0.92f, 0.88f);

    // ── Lambda: draw one full player ──────────────────────────────────────
    // px,pz      : world position
    // racketSide : +1 = racket arm on +X side (P1 faces north)
    //              -1 = racket arm on -X side (P2 faces south)
    // armAngle   : current swing angle
    // lean       : forward lean offset along Z toward net
    // shirt/racket colours
    auto drawPlayer = [&](float px, float pz,
        int racketSide,
        float armAngle,
        float lean,
        const glm::vec3& shirt,
        const glm::vec3& racket)
        {
            const float BY = BC_Y;
            glm::mat4 m;

            // Lean shifts the upper body toward the net (net is at BC_Z)
            float leanDirZ = (pz < BC_Z) ? lean : -lean;   // P1 leans forward (+Z), P2 backward (-Z)

            // ── Shoes ─────────────────────────────────────────────────────────
            for (int s : {-1, 1}) {
                m = glm::scale(glm::translate(glm::mat4(1.f),
                    { px + s * 0.18f, BY + 0.12f, pz }),
                    { 0.22f, 0.22f, 0.42f });
                drawCube(sh, m, SHOE);
            }

            // ── Legs / shorts ──────────────────────────────────────────────────
            for (int s : {-1, 1}) {
                m = glm::scale(glm::translate(glm::mat4(1.f),
                    { px + s * 0.18f, BY + 0.65f, pz }),
                    { 0.22f, 0.80f, 0.22f });
                drawCube(sh, m, SHORTS);
            }

            // ── Hip block ─────────────────────────────────────────────────────
            m = glm::scale(glm::translate(glm::mat4(1.f),
                { px, BY + 1.02f, pz }),
                { 0.52f, 0.22f, 0.32f });
            drawCube(sh, m, SHORTS);

            // ── Torso (with forward lean) ──────────────────────────────────────
            m = glm::scale(glm::translate(glm::mat4(1.f),
                { px, BY + 1.55f, pz + leanDirZ }),
                { 0.55f, 0.72f, 0.34f });
            drawCube(sh, m, shirt, texConcrete, 1.f);

            // ── Head ──────────────────────────────────────────────────────────
            m = glm::scale(glm::translate(glm::mat4(1.f),
                { px, BY + 2.20f, pz + leanDirZ }),
                { 0.42f, 0.42f, 0.42f });
            drawSphere(sh, m, SKIN, texMarble, 1.f);

            // Hair
            m = glm::scale(glm::translate(glm::mat4(1.f),
                { px, BY + 2.50f, pz + leanDirZ }),
                { 0.44f, 0.22f, 0.44f });
            drawSphere(sh, m, glm::vec3(0.12f, 0.08f, 0.05f));

            // ── Racket (dominant) arm ─────────────────────────────────────────
            {
                float rax = px + racketSide * 0.38f;
                glm::mat4 armBase = glm::translate(glm::mat4(1.f),
                    { rax, BY + 1.62f, pz + leanDirZ * 0.5f });
                // Rotate about X to swing racket forward/back
                armBase = glm::rotate(armBase, glm::radians(armAngle), { 1.f, 0.f, 0.f });

                // Upper arm
                m = glm::scale(armBase, { 0.14f, 0.50f, 0.14f });
                drawCube(sh, m, SKIN);

                // Forearm
                glm::mat4 forearmBase = glm::translate(armBase, { 0.f, 0.50f, 0.f });
                forearmBase = glm::rotate(forearmBase,
                    glm::radians(armAngle * 0.45f), { 1.f, 0.f, 0.f });
                m = glm::scale(forearmBase, { 0.13f, 0.45f, 0.13f });
                drawCube(sh, m, SKIN);

                // Grip / handle
                glm::mat4 handleBase = glm::translate(forearmBase, { 0.f, 0.46f, 0.f });
                m = glm::scale(handleBase, { 0.06f, 0.40f, 0.06f });
                drawCylinder(sh, m, HANDLE_COL);

                // Racket head frame
                glm::mat4 racketHead = glm::translate(handleBase, { 0.f, 0.52f, 0.f });
                m = glm::scale(racketHead, { 0.38f, 0.52f, 0.04f });
                drawCube(sh, m, racket);

                // Strings
                m = glm::scale(racketHead, { 0.30f, 0.44f, 0.045f });
                drawCube(sh, m, STRING_COL);
            }

            // ── Free (non-racket) arm – balanced, slight forward raise ─────────
            {
                float fax = px - racketSide * 0.38f;  // opposite side
                // Free arm counters the swing: rises slightly when racket arm swings
                float freeAngle = -15.f - armAngle * 0.25f;  // counter-balance
                freeAngle = glm::clamp(freeAngle, -60.f, 20.f);

                glm::mat4 fArmBase = glm::translate(glm::mat4(1.f),
                    { fax, BY + 1.62f, pz + leanDirZ * 0.5f });
                fArmBase = glm::rotate(fArmBase, glm::radians(freeAngle), { 1.f, 0.f, 0.f });

                m = glm::scale(fArmBase, { 0.14f, 0.50f, 0.14f });
                drawCube(sh, m, SKIN);

                glm::mat4 fForearm = glm::translate(fArmBase, { 0.f, 0.50f, 0.f });
                fForearm = glm::rotate(fForearm, glm::radians(-15.f), { 1.f, 0.f, 0.f });
                m = glm::scale(fForearm, { 0.13f, 0.44f, 0.13f });
                drawCube(sh, m, SKIN);
            }
        };

    // ── Player 1: south half, faces net (north), racket arm on +X ─────────
    drawPlayer(BC_X + sway1, BC_Z - 3.5f,
        +1, arm1Angle, lean1,
        SHIRT1, RACKET1);

    // ── Player 2: north half, faces net (south), racket arm on -X ─────────
    drawPlayer(BC_X + sway2, BC_Z + 3.5f,
        -1, arm2Angle, lean2,
        SHIRT2, RACKET2);
}

// ════════════════════════════════════════════════════════════════════════════
//  renderShuttle  (unchanged from original)
// ════════════════════════════════════════════════════════════════════════════
static void renderShuttle(unsigned int sh) {
    float t = (float)glfwGetTime();

    const float PERIOD = 2.0f;
    float phase = fmodf(t, PERIOD) / PERIOD;

    float halfPhase, dirSign;
    if (phase < 0.5f) { halfPhase = phase * 2.f;          dirSign = 1.f; }
    else { halfPhase = (phase - 0.5f) * 2.f; dirSign = -1.f; }

    const float ZS = BC_Z - 3.0f;
    const float ZN = BC_Z + 3.0f;
    float shuttleZ = ZS + (ZN - ZS) * halfPhase;

    const float BASE_Y = 1.1f;
    const float PEAK_Y = 3.8f;
    float yNorm = 4.f * halfPhase * (1.f - halfPhase);
    float shuttleY = BASE_Y + (PEAK_Y - BASE_Y) * yNorm;
    float shuttleX = BC_X + 0.4f * sinf(t * 1.2f);
    float spinAngle = t * 360.f;

    glm::mat4 base = glm::translate(glm::mat4(1.f), { shuttleX, shuttleY, shuttleZ });
    base = glm::rotate(base, glm::radians(spinAngle), { 0.f, 1.f, 0.f });
    base = glm::rotate(base, glm::radians(dirSign * 90.f + 30.f), { 1.f, 0.f, 0.f });

    // Cork base
    glm::mat4 m = glm::scale(base, { 0.12f, 0.12f, 0.12f });
    drawSphere(sh, m, glm::vec3(0.95f, 0.92f, 0.88f), texMarble, 1.f);

    // Feather cone
    m = glm::scale(base, { 0.30f, 0.30f, 0.30f });
    m = glm::translate(m, { 0.f, 0.5f, 0.f });
    drawCone(sh, m, glm::vec3(0.96f, 0.94f, 0.90f));

    // Ground shadow
    float shadowScale = 0.15f + 0.10f * (1.f - yNorm);
    glm::mat4 shadow = glm::scale(
        glm::translate(glm::mat4(1.f), { shuttleX, BC_Y + 0.01f, shuttleZ }),
        { shadowScale * 2.5f, 0.02f, shadowScale * 2.5f });
    drawCylinder(sh, shadow, glm::vec3(0.05f, 0.10f, 0.05f));
}

// ════════════════════════════════════════════════════════════════════════════
//  renderScene()
// ════════════════════════════════════════════════════════════════════════════
void updateDiningHallFan(float deltaTime);
void renderScene(unsigned int sh) {

    // ── ROOM MODE: render room only, then stop ────────────────────────────
    if (inRoomView) {
        renderRoom(sh);
        return;
    }

    // ── CAMPUS MODE ───────────────────────────────────────────────────────
    renderGround(sh);
    renderBoundaryWall(sh);
    renderDormBlockA(sh);
    renderCorridor(sh);
    renderDormBlockB(sh);
    renderCourtyard(sh);
    renderAdminAnnex(sh);
    renderEntranceSphere(sh);
    renderBadmintonCourt(sh);

    // Badminton: use the SAME period as renderShuttle (2.0 s)
    float t = (float)glfwGetTime();
    float phase = fmodf(t, 2.0f) / 2.0f;
    renderBadmintonPlayers(sh, phase);
    renderShuttle(sh);

    renderCricketPitch(sh);
    renderBatsman(sh);
    renderCricketBall(sh);
    renderPaths(sh);
    renderFractalTrees(sh);
    renderTelevision(sh);
    renderGate(sh);
    renderBezierObjects(sh);
    drawSplineRailing(sh);
    drawRuledSurfaceRoof(sh);
}