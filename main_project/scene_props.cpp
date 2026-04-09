#include "globals.h"
#include "draw_helpers.h"
#include "scene_ground.h"
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
// ════════════════════════════════════════════════════════════════════════════
//  Fractal tree renderer
// ════════════════════════════════════════════════════════════════════════════
static void drawFractalBranch(
    unsigned int sh,
    glm::mat4    transform,
    float        length,
    float        radius,
    int          depth,
    int          maxDepth);

static void drawFractalTreeCustom(unsigned int sh,
    float wx, float wy, float wz,
    float length, float radius, int depth)
{
    glm::mat4 base = glm::translate(glm::mat4(1.f), glm::vec3(wx, wy, wz));
    drawFractalBranch(sh, base, length, radius, depth, depth);
}

static void drawFractalBranch(
    unsigned int sh,
    glm::mat4    transform,   // current branch's coordinate frame
    float        length,      // length of this branch segment
    float        radius,      // radius of this branch cylinder
    int          depth,       // remaining recursion levels
    int          maxDepth)
{
    if (depth < 0) return;

    // ── Draw the trunk / branch cylinder ──────────────────────────────────
    // Cylinder is 1 unit tall centred at origin in local space.
    // We stretch it to (radius*2 × length × radius*2) and translate by
    // half-length so its base sits at the local origin.
    const glm::vec3 BARK(0.36f, 0.22f, 0.10f);   // brown bark
    glm::mat4 cyl = glm::translate(transform, glm::vec3(0.f, length * 0.5f, 0.f));
    cyl = glm::scale(cyl, glm::vec3(radius * 2.f, length, radius * 2.f));
    drawCylinder(sh, cyl, BARK);

    if (depth == 0) {
        // Leaf tuft: green cone at tip
        const glm::vec3 LEAF(0.18f, 0.55f, 0.15f);
        glm::mat4 leaf = glm::translate(transform, glm::vec3(0.f, length, 0.f));
        leaf = glm::scale(leaf, glm::vec3(radius * 5.f, radius * 9.f, radius * 5.f));
        drawCone(sh, leaf, LEAF);
        return;
    }

    // ── Move to tip of current branch ─────────────────────────────────────
    glm::mat4 tip = glm::translate(transform, glm::vec3(0.f, length, 0.f));

    float childLen = length * 0.68f;
    float childRadius = radius * 0.62f;
    int   childDepth = depth - 1;

    // Two children: lean left and right, plus one straighter centre branch
    struct BranchDef { float rotY; float tiltX; };
    BranchDef branches[] = {
        { -40.f,  28.f },   // left fork
        {  40.f,  28.f },   // right fork
        {   0.f,  14.f },   // centre (slight forward lean)
    };

    for (auto& b : branches) {
        glm::mat4 t = glm::rotate(tip,
            glm::radians(b.rotY), glm::vec3(0.f, 1.f, 0.f));
        t = glm::rotate(t,
            glm::radians(b.tiltX), glm::vec3(1.f, 0.f, 0.f));
        drawFractalBranch(sh, t, childLen, childRadius, childDepth, maxDepth);
    }
}

void renderFractalTrees(unsigned int sh)
{
    // Move both trees to near front boundary corners, inside the wall.
    drawFractalTreeCustom(sh, -29.0f, 0.f, 31.0f, 5.5f, 0.5f, 6);
    drawFractalTreeCustom(sh,  29.0f, 0.f, 31.0f, 5.5f, 0.5f, 6);
}
// ── Palm tree ────────────────────────────────────────────────────────────────
static void renderPalmTree(unsigned int sh, glm::vec3 base) {
    for (int i = 0; i < 5; i++) {
        float y = base.y + 0.5f + i, sc = 0.20f - i * 0.015f;
        glm::mat4 m = glm::translate(glm::mat4(1.f), { base.x,y,base.z });
        m = glm::rotate(m, glm::radians((float)(i * 4)), { 0,1,0 });
        m = glm::scale(m, { sc,1.f,sc });
        m = glm::rotate(m, glm::radians(90.f), { 1,0,0 });
        drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    }
    for (int i = 0; i < 6; i++) {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { base.x,base.y + 5.5f,base.z });
        m = glm::rotate(m, glm::radians(i * 60.f), { 0,1,0 });
        m = glm::translate(m, { 1.2f,0.3f,0.f });
        m = glm::rotate(m, glm::radians(-30.f), { 0,0,1 });
        m = glm::scale(m, { 2.5f,0.12f,0.5f });
        drawCube(sh, m, COL_PALM, texGrass, 1.f);
    }
}

// ── Evergreen cone trees ──────────────────────────────────────────────────────
static void renderConeTree(unsigned int sh, float x, float z) {
    glm::mat4 m = glm::translate(glm::mat4(1.f), { x,1.5f,z });
    m = glm::rotate(m, glm::radians(90.f), { 1,0,0 });
    m = glm::scale(m, { 0.4f,0.4f,3.f });
    drawCylinder(sh, m, COL_TRUNK, texWood, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,4.5f,z }), { 3.5f,4.5f,3.5f });
    drawCone(sh, m, COL_PALM, texGrass, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,7.f,z }), { 2.3f,3.5f,2.3f });
    drawCone(sh, m, COL_PALM, texGrass, 1.5f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,9.f,z }), { 1.2f,2.5f,1.2f });
    drawCone(sh, m, COL_PALM, texGrass, 1.f);
}

// ── Bench ────────────────────────────────────────────────────────────────────
static void renderBench(unsigned int sh, glm::vec3 pos, float rotY) {
    glm::mat4 base = glm::rotate(glm::translate(glm::mat4(1.f), pos), glm::radians(rotY), { 0,1,0 });
    // seat
    glm::mat4 m = glm::scale(base, { 2.0f,0.12f,0.5f });
    m = glm::translate(m, { 0.f,5.5f,0.f });
    drawCube(sh, glm::scale(glm::translate(base, { 0.f,0.66f,0.f }), { 2.0f,0.12f,0.5f }), COL_TRUNK, texWood, 1.f);
    // legs
    for (int s : {-1, 1}) {
        drawCube(sh, glm::scale(glm::translate(base, { s * 0.8f,0.33f,0.18f }), { 0.1f,0.66f,0.1f }), COL_STONE);
        drawCube(sh, glm::scale(glm::translate(base, { s * 0.8f,0.33f,-0.18f }), { 0.1f,0.66f,0.1f }), COL_STONE);
    }
    // back rest
    drawCube(sh, glm::scale(glm::translate(base, { 0.f,1.1f,-0.22f }), { 2.0f,0.5f,0.08f }), COL_TRUNK, texWood, 1.f);
}

// ── Fountain ─────────────────────────────────────────────────────────────────
static void renderFountain(unsigned int sh, glm::vec3 pos) {
    glm::mat4 m;
    // basin ring
    m = glm::scale(glm::translate(glm::mat4(1.f), pos), { 4.5f,0.5f,4.5f });
    drawCylinder(sh, m, COL_MARBLE, texMarble, 2.f);
    // inner water surface (flat sphere stand-in)
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 0.28f,pos.z }), { 3.6f,0.15f,3.6f });
    drawCube(sh, m, COL_WATER);
    // central pedestal
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 0.25f,pos.z }), { 0.6f,1.8f,0.6f });
    drawCylinder(sh, m, COL_MARBLE, texMarble, 1.f);
    // top sphere
    m = glm::scale(glm::translate(glm::mat4(1.f), { pos.x,pos.y + 1.8f,pos.z }), { 1.5f,1.5f,1.5f });
    drawSphere(sh, m, COL_MARBLE, texMarble, 2.f);
}

// ── Inner Courtyard ───────────────────────────────────────────────────────────
void renderCourtyard(unsigned int sh) {
    renderFountain(sh, { -4.f, 0.3f, -12.f });
    renderBench(sh, { -12.f,0.7f,-9.f }, 10.f);
    renderBench(sh, { 4.f, 0.7f,-9.f }, -10.f);
    renderBench(sh, { -12.f,0.7f,-15.f }, 170.f);
    renderBench(sh, { 4.f, 0.7f,-15.f }, 190.f);
    // Low hedge / garden beds
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { -18.f,0.35f,-12.f }), { 3.f,0.7f,8.f });
    drawCube(sh, m, glm::vec3(0.18f, 0.45f, 0.12f), texGrass, 2.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { 10.f,0.35f,-12.f }), { 3.f,0.7f,8.f });
    drawCube(sh, m, glm::vec3(0.18f, 0.45f, 0.12f), texGrass, 2.f);
}

static void renderLampPost(unsigned int sh, float x, float z) {
    glm::mat4 m;
    // Pole
    m = glm::scale(glm::translate(glm::mat4(1.f), { x,3.5f,z }), { 0.15f,7.f,0.15f });
    drawCylinder(sh, m, glm::vec3(0.35f, 0.35f, 0.36f), texConcrete, 1.f);
    // Arm
    m = glm::scale(glm::translate(glm::mat4(1.f), { x + 0.6f,7.1f,z }), { 1.2f,0.12f,0.12f });
    drawCube(sh, m, glm::vec3(0.35f, 0.35f, 0.36f));
    // Lamp head
    m = glm::scale(glm::translate(glm::mat4(1.f), { x + 1.2f,7.0f,z }), { 0.4f,0.35f,0.4f });
    drawCylinder(sh, m, glm::vec3(0.95f, 0.90f, 0.60f));
    // Light cone glow
    if (!dayMode) {
        static const glm::vec3 glow(1.0f, 0.95f, 0.70f);
        static const glm::vec3 em(0.9f, 0.85f, 0.55f);
        static const glm::vec3 zero(0.f, 0.f, 0.f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(em));
        m = glm::scale(glm::translate(glm::mat4(1.f), { x + 1.2f,6.5f,z }), { 0.5f,0.5f,0.5f });
        drawCone(sh, m, glow);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(zero));
    }
}
void renderEntranceSphere(unsigned int sh) {
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.f), { -20.f,0.5f,18.f }), { 1.6f,1.0f,1.6f });
    drawCube(sh, m, COL_STONE, texConcrete, 1.f);
    m = glm::translate(glm::mat4(1.f), { -20.f,1.6f,18.f });
    m = glm::rotate(m, glm::radians(90.f), { 1,0,0 });
    m = glm::scale(m, { 0.5f,0.5f,1.2f });
    drawCylinder(sh, m, COL_STONE, texConcrete, 1.f);
    m = glm::scale(glm::translate(glm::mat4(1.f), { -20.f,4.0f,18.f }), { 4.5f,4.5f,4.5f });
    drawSphere(sh, m, COL_MARBLE, texMarble, 1.f);
}
void renderTelevision(unsigned int sh) {

    float t = (float)glfwGetTime();
    static const glm::vec3 ZERO3(0.f, 0.f, 0.f);

    // ─── TV anchor position ───────────────────────────────────────────────
    //   screen centre in world space
    const float TV_X = 6.0f;   // right of central path
    const float TV_Y = 2.5f;   // screen centre Y (≈ 2.5 m above ground)
    const float TV_Z = -5.0f;   // courtyard between Block-A and corridor

    // Screen face at x = TV_X - 0.14  (faces west / -X)
    const float FACE_X = TV_X - 0.142f;

    // ─── Screen dimensions ────────────────────────────────────────────────
    const float SCR_W = 3.2f;   // width  (Z axis)
    const float SCR_H = 1.9f;   // height (Y axis)
    const float BEZ = 0.18f;  // bezel margin around screen

    glm::mat4 m;

    // ═══════════════════════════════════════════════════════════════════════
    // 1.  STRUCTURE
    // ═══════════════════════════════════════════════════════════════════════

    // ── Outer bezel (jet black, includes BEZ margin on all sides) ─────────
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { TV_X, TV_Y, TV_Z }),
        { 0.28f, SCR_H + BEZ * 2.f, SCR_W + BEZ * 2.f });
    drawCube(sh, m, glm::vec3(0.07f, 0.07f, 0.08f));

    // ── Thin inner bezel rim (dark-gray, 3mm proud of screen face) ────────
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X + 0.008f, TV_Y, TV_Z }),
        { 0.05f, SCR_H + 0.06f, SCR_W + 0.06f });
    drawCube(sh, m, glm::vec3(0.15f, 0.15f, 0.17f));

    // ── Trolley stand pole ────────────────────────────────────────────────
    // Vertical neck
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { TV_X, TV_Y - SCR_H * 0.5f - 0.90f, TV_Z }),
        { 0.12f, 1.80f, 0.12f });
    drawCylinder(sh, m, glm::vec3(0.25f, 0.25f, 0.27f));

    // Horizontal base bar (front-back)
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { TV_X, TV_Y - SCR_H * 0.5f - 1.78f, TV_Z }),
        { 0.90f, 0.12f, 0.12f });
    drawCube(sh, m, glm::vec3(0.22f, 0.22f, 0.24f));

    // Horizontal base bar (left-right)
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { TV_X, TV_Y - SCR_H * 0.5f - 1.78f, TV_Z }),
        { 0.12f, 0.12f, 1.20f });
    drawCube(sh, m, glm::vec3(0.22f, 0.22f, 0.24f));

    // Four caster wheels
    for (int sz : {-1, 1}) for (int sx : {-1, 1}) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { TV_X + sx * 0.38f,
                 TV_Y - SCR_H * 0.5f - 1.82f,
                 TV_Z + sz * 0.52f }),
            { 0.14f, 0.14f, 0.14f });
        drawSphere(sh, m, glm::vec3(0.12f, 0.12f, 0.12f));
    }

    // Speaker bar (narrow black bar beneath screen)
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { FACE_X - 0.01f,
             TV_Y - SCR_H * 0.5f - 0.14f,
             TV_Z }),
        { 0.10f, 0.20f, SCR_W * 0.55f });
    drawCube(sh, m, glm::vec3(0.10f, 0.10f, 0.12f));

    // Speaker grille dots on bar
    for (int d = -4; d <= 4; d++) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { FACE_X - 0.005f,
                 TV_Y - SCR_H * 0.5f - 0.14f,
                 TV_Z + d * 0.20f }),
            { 0.05f, 0.06f, 0.06f });
        drawCylinder(sh, m, glm::vec3(0.30f, 0.30f, 0.32f));
    }

    // Power indicator LED (tiny green dot, always emissive)
    {
        glm::vec3 led(0.10f, 0.90f, 0.20f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(led));
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { FACE_X - 0.005f,
                 TV_Y - SCR_H * 0.5f - 0.02f,
                 TV_Z - SCR_W * 0.5f + 0.20f }),
            { 0.04f, 0.06f, 0.06f });
        drawCube(sh, m, led);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // ═══════════════════════════════════════════════════════════════════════
    // 2.  ANIMATED SCREEN
    //     16-second master cycle → 4 broadcast scenes of 4 s each.
    //     Screen face is the -X face of a thin slab placed at x = FACE_X.
    //     Every colour and stripe uses glUniform "emissive" so the screen
    //     ALWAYS glows, completely independent of scene lighting.
    // ═══════════════════════════════════════════════════════════════════════

    float period = fmodf(t, 16.f);   // 0..16
    float sceneFrac = fmodf(t, 4.f) / 4.f;  // 0..1 within scene

    // ── Per-scene colour palette ──────────────────────────────────────────
    glm::vec3 bgColor, stripe1, stripe2, tickerColor;
    float     stripe1Y, stripe2Y;   // Y offsets of horizontal stripes on screen
    float     movingDotZ;           // animated dot Z on screen

    if (period < 4.f) {
        // Scene 0 – CRICKET BROADCAST (green outfield + red ball close-up)
        float g = 0.32f + 0.10f * sinf(t * 0.7f);
        bgColor = { 0.06f, g,    0.04f };          // green pitch
        stripe1 = { 0.92f, 0.90f, 0.85f };         // scoreboard white
        stripe2 = { 0.80f, 0.12f, 0.08f };         // red ball flash
        stripe1Y = TV_Y + SCR_H * 0.35f;          // top band (scoreboard)
        stripe2Y = TV_Y - SCR_H * 0.38f;          // bottom band (ticker)
        tickerColor = { 0.90f, 0.75f, 0.10f };         // gold ticker
        movingDotZ = TV_Z + (SCR_W * 0.38f) * sinf(t * 1.4f);  // ball tracking dot

    }
    else if (period < 8.f) {
        // Scene 1 – FOOTBALL MATCH (darker green, moving white lines)
        float gv = 0.36f + 0.06f * cosf(t * 1.2f);
        bgColor = { 0.05f, gv,   0.07f };
        stripe1 = { 0.95f, 0.95f, 0.92f };         // pitch line
        stripe2 = { 0.20f, 0.50f, 0.90f };         // blue sky background hint
        stripe1Y = TV_Y + SCR_H * 0.30f * sinf(t * 0.6f);  // moving centre line
        stripe2Y = TV_Y + SCR_H * 0.40f;
        tickerColor = { 0.88f, 0.20f, 0.20f };         // red live ticker
        movingDotZ = TV_Z + (SCR_W * 0.42f) * cosf(t * 2.0f);  // ball tracking

    }
    else if (period < 12.f) {
        // Scene 2 – NEWS STUDIO (blue background, lower-third graphic)
        float bv = 0.45f + 0.15f * sinf(t * 0.35f);
        bgColor = { 0.04f, 0.10f, bv };             // deep blue studio
        stripe1 = { 0.96f, 0.88f, 0.70f };          // warm presenter highlight
        stripe2 = { 0.88f, 0.18f, 0.18f };          // BREAKING NEWS red band
        stripe1Y = TV_Y + SCR_H * 0.05f;           // mid-screen divider
        stripe2Y = TV_Y - SCR_H * 0.38f;           // lower-third (fixed)
        tickerColor = { 0.96f, 0.96f, 0.92f };          // white scrolling text
        movingDotZ = TV_Z + (SCR_W * 0.48f) * sinf(t * 0.40f);  // slow text scroll

    }
    else {
        // Scene 3 – BADMINTON MATCH (bright court, fast shuttle dot)
        float fl = 0.78f + 0.10f * sinf(t * 6.f);  // court light flicker
        bgColor = { fl * 0.82f, fl * 0.84f, fl * 0.90f };
        stripe1 = { 0.96f, 0.96f, 0.94f };           // court line white
        stripe2 = { 0.20f, 0.48f, 0.88f };           // blue jersey band
        stripe1Y = TV_Y;                             // centre net line
        stripe2Y = TV_Y + SCR_H * 0.32f;
        tickerColor = { 0.88f, 0.20f, 0.88f };           // pink overlay ticker
        movingDotZ = TV_Z + (SCR_W * 0.44f) * sinf(t * 3.5f);  // fast shuttle
    }

    // ── 2a. Full screen background (emissive slab, -X face visible) ───────
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(bgColor));
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X, TV_Y, TV_Z }),
        { 0.05f, SCR_H, SCR_W });
    drawCube(sh, m, bgColor);

    // ── 2b. Horizontal stripe 1 (band across screen) ─────────────────────
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(stripe1 * 0.90f));
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X, stripe1Y, TV_Z }),
        { 0.052f, SCR_H * 0.12f, SCR_W });
    drawCube(sh, m, stripe1);

    // ── 2c. Horizontal stripe 2 ───────────────────────────────────────────
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(stripe2 * 0.90f));
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X, stripe2Y, TV_Z }),
        { 0.052f, SCR_H * 0.09f, SCR_W });
    drawCube(sh, m, stripe2);

    // ── 2d. Animated ticker / lower-third wide bar ────────────────────────
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(tickerColor * 0.85f));
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { FACE_X, TV_Y - SCR_H * 0.44f, TV_Z }),
        { 0.053f, SCR_H * 0.10f, SCR_W * 0.80f });
    drawCube(sh, m, tickerColor);

    // ── 2e. Moving bright dot (ball / player / graphics object) ──────────
    float dotY = TV_Y + SCR_H * 0.15f * sinf(t * 1.85f);
    glm::vec3 dotCol(0.98f, 0.98f, 0.96f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(dotCol));
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X, dotY, movingDotZ }),
        { 0.055f, SCR_H * 0.10f, SCR_H * 0.10f });
    drawCube(sh, m, dotCol);

    // ── 2f. Scan-line shimmer (very thin bright bar sweeping top→bottom) ──
    float scanY = TV_Y + (SCR_H * 0.5f) - fmodf(t * 1.5f, SCR_H);
    glm::vec3 scanCol(1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(scanCol * 0.35f));
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { FACE_X, scanY, TV_Z }),
        { 0.055f, 0.04f, SCR_W });
    drawCube(sh, m, scanCol);

    // ── Always reset emissive ─────────────────────────────────────────────
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));

    // ═══════════════════════════════════════════════════════════════════════
    // 3.  SCREEN GLOW  –  coloured light spill on floor/wall around TV
    //     Drawn as large very-dim cubes so the area around the TV looks lit.
    // ═══════════════════════════════════════════════════════════════════════
    glm::vec3 glowCol = bgColor * 0.28f;
    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(glowCol));

    // Floor glow patch
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { TV_X - 2.0f, 0.05f, TV_Z }),
        { 4.0f, 0.06f, SCR_W + 2.f });
    drawCube(sh, m, bgColor);

    // Wall/air glow slab to the left (west) of TV
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { TV_X - 3.0f, TV_Y, TV_Z }),
        { 0.06f, SCR_H + 1.0f, SCR_W + 2.f });
    drawCube(sh, m, bgColor);

    glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
}
void renderGate(unsigned int sh) {

    static const float GZ = 35.f;    // gate Z position (in front of boundary wall)
    static const float GX = -5.f;   // gate centre X  (aligns with central path)
    static const float GAP = 5.2f;   // half-gap between inner pillar faces (total 10.4 m opening)
    static const float PW = 1.6f;   // pillar width / depth
    static const float PHT = 6.5f;   // pillar height
    static const float WING_L = 8.f;   // side wing wall length (outward from each pillar)
    static const float WING_H = 3.0f;  // side wing wall height

    glm::mat4 m;
    static const glm::vec3 ZERO3(0.f, 0.f, 0.f);

    // ── Side wing walls (left & right of pillars, same brick as boundary) ─
    // Left wing: extends from left pillar outward in -X
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { GX - GAP - PW * 0.5f - WING_L * 0.5f, WING_H * 0.5f, GZ }),
        { WING_L, WING_H, 0.55f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);
    // Right wing: extends from right pillar outward in +X
    m = glm::scale(
        glm::translate(glm::mat4(1.f),
            { GX + GAP + PW * 0.5f + WING_L * 0.5f, WING_H * 0.5f, GZ }),
        { WING_L, WING_H, 0.55f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 4.f);

    // ── Main gate pillars (left & right) ──────────────────────────────────
    for (int s : {-1, 1}) {
        float px = GX + s * GAP;

        // Plinth base (wider than pillar, sits on ground)
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { px, 0.3f, GZ }),
            { PW + 0.6f, 0.60f, PW + 0.6f });
        drawCube(sh, m, COL_STONE, texConcrete, 1.f);

        // Main shaft (tall concrete/stone pillar)
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { px, PHT * 0.5f + 0.6f, GZ }),
            { PW, PHT, PW });
        drawCube(sh, m, COL_COLUMN, texConcrete, 2.f);

        // Vertical grooved lines (decorative inset strips, 3 per pillar)
        for (int g = -1; g <= 1; g++) {
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { px + g * (PW * 0.28f), PHT * 0.5f + 0.6f, GZ - PW * 0.49f }),
                { 0.08f, PHT - 0.2f, 0.04f });
            drawCube(sh, m, glm::vec3(0.70f, 0.70f, 0.68f));
        }

        // Capital block (top of shaft)
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { px, PHT + 0.6f + 0.25f, GZ }),
            { PW + 0.35f, 0.50f, PW + 0.35f });
        drawCube(sh, m, COL_COLUMN, texConcrete, 1.f);

        // Sphere finial (marble ball on top of each pillar)
        m = glm::scale(
            glm::translate(glm::mat4(1.f), { px, PHT + 0.6f + 0.50f + 0.70f, GZ }),
            { 1.40f, 1.40f, 1.40f });
        drawSphere(sh, m, COL_MARBLE, texMarble, 1.f);

        // Emissive night light ring under finial sphere
        if (!dayMode) {
            glm::vec3 glow(0.85f, 0.80f, 0.45f);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(glow * 0.7f));
            m = glm::scale(
                glm::translate(glm::mat4(1.f),
                    { px, PHT + 0.6f + 0.22f, GZ }),
                { PW + 0.4f, 0.12f, PW + 0.4f });
            drawCylinder(sh, m, glow);
            glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
        }
    }

    // ── Arch beam spanning both pillars ───────────────────────────────────
    // Main horizontal lintel
    float lintY = PHT + 0.6f + 0.25f;
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { GX, lintY + 0.75f, GZ }),
        { GAP * 2.f + PW * 2.f + 0.35f, 0.80f, PW + 0.35f });
    drawCube(sh, m, COL_COLUMN, texConcrete, 2.f);

    // Decorative keystone block at arch centre
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { GX, lintY + 1.40f, GZ }),
        { 1.8f, 0.80f, PW + 0.40f });
    drawCube(sh, m, COL_STONE, texConcrete, 1.f);

    // Three arch corbels (evenly spaced along beam underside)
    for (int c = -1; c <= 1; c++) {
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { GX + c * (GAP * 0.6f), lintY + 0.30f, GZ }),
            { 0.50f, 0.30f, PW + 0.38f });
        drawCube(sh, m, glm::vec3(0.68f, 0.67f, 0.65f), texConcrete, 1.f);
    }

    // ── Gate name plate (emissive glowing sign between pillars) ───────────
    // Dark backing panel
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { GX, lintY + 0.72f, GZ - PW * 0.38f }),
        { GAP * 1.2f, 0.55f, 0.10f });
    drawCube(sh, m, glm::vec3(0.08f, 0.08f, 0.10f));

    // Name-plate emissive face (gold tone)
    {
        glm::vec3 np(0.90f, 0.78f, 0.30f);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(np * 0.70f));
        m = glm::scale(
            glm::translate(glm::mat4(1.f),
                { GX, lintY + 0.72f, GZ - PW * 0.39f }),
            { GAP * 1.18f, 0.50f, 0.05f });
        drawCube(sh, m, np);
        glUniform3fv(glGetUniformLocation(sh, "emissive"), 1, glm::value_ptr(ZERO3));
    }

    // Gate opening intentionally left clear (no center stand / no gate panels)

    // ── Guard booth (small cube structure beside left pillar) ─────────────
    float boothX = GX - GAP - PW * 0.5f - 2.2f;
    // Booth body
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { boothX, 1.20f, GZ }),
        { 2.0f, 2.4f, 2.0f });
    drawCube(sh, m, glm::vec3(0.78f, 0.76f, 0.72f), texConcrete, 1.f);
    // Booth roof
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { boothX, 2.55f, GZ }),
        { 2.3f, 0.25f, 2.3f });
    drawCube(sh, m, COL_REDBRICK, texBrick, 1.f);
    // Booth window
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { boothX, 1.50f, GZ + 1.05f }),
        { 1.0f, 0.80f, 0.12f });
    drawCrystalGlass(sh, m);
    // Booth door (front face)
    m = glm::scale(
        glm::translate(glm::mat4(1.f), { boothX, 0.90f, GZ - 1.05f }),
        { 0.80f, 1.80f, 0.12f });
    drawCube(sh, m, COL_DOOR, texWood, 1.f);
}
