#include "washroom.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>
#include <cmath>

static unsigned int wash_sh = 0;
static float washFanAngle = 0.0f;
static bool washLightOn = true;

// ── Bezier-surface VAOs ──────────────────────────────────────────────────────
static unsigned int toiletBowlVAO = 0; static int toiletBowlIdx = 0;
static unsigned int toiletTankVAO = 0; static int toiletTankIdx = 0;
static unsigned int sinkBasinVAO = 0; static int sinkBasinIdx = 0;
static unsigned int sinkPedestalVAO = 0; static int sinkPedestalIdx = 0;
static unsigned int bathtubVAO = 0; static int bathtubIdx = 0;
static unsigned int showerHeadVAO = 0; static int showerHeadIdx = 0;
static unsigned int faucetVAO = 0; static int faucetIdx = 0;
static bool washroomInitDone = false;

static std::vector<float>        bezVerts;
static std::vector<unsigned int> bezIdxs;

// ── Room constants ───────────────────────────────────────────────────────────
static const float WASH_X = 5.f;
static const float WASH_Z = 18.f;
static const float WASH_W = 8.f;   // full width  (±4 from WASH_X)
static const float WASH_D = 10.f;   // full depth  (±5 from WASH_Z)
static const float WASH_H = 4.0f;  // ceiling height

// ── Helpers ──────────────────────────────────────────────────────────────────
static void washBox(float cx, float cy, float cz,
    float sx, float sy, float sz,
    glm::vec3 col,
    unsigned int tex = 0, float tile = 1.f)
{
    glm::mat4 m = glm::translate(glm::mat4(1.f), { cx, cy, cz });
    m = glm::scale(m, { sx, sy, sz });
    drawCube(wash_sh, m, col, tex, tile);
}

static void setColor(glm::vec3 c) {
    glUniform3fv(glGetUniformLocation(wash_sh, "objectColor"), 1, glm::value_ptr(c));
}
static void setModel(glm::mat4 m) {
    glUniformMatrix4fv(glGetUniformLocation(wash_sh, "model"), 1, GL_FALSE, glm::value_ptr(m));
}
static void setEmissive(glm::vec3 e) {
    glUniform3fv(glGetUniformLocation(wash_sh, "emissive"), 1, glm::value_ptr(e));
}
static void noTexture() {
    glUniform1i(glGetUniformLocation(wash_sh, "hasTexture"), 0);
}

// ── Lighting ─────────────────────────────────────────────────────────────────
static void washLighting()
{
    // Warm-white main overhead + cool fill from mirror wall
    glm::vec3 amb = washLightOn
        ? glm::vec3(0.28f, 0.27f, 0.25f)
        : glm::vec3(0.04f, 0.04f, 0.05f);
    glUniform3fv(glGetUniformLocation(wash_sh, "ambientColor"), 1, glm::value_ptr(amb));

    if (washLightOn) {
        // Main ceiling panel – warm white
        glUniform3f(glGetUniformLocation(wash_sh, "lightPos"), WASH_X, WASH_H - 0.15f, WASH_Z);
        glUniform3f(glGetUniformLocation(wash_sh, "lightColor"), 1.00f, 0.96f, 0.90f);
        glUniform1f(glGetUniformLocation(wash_sh, "lightIntensity"), 1.4f);
        // Vanity strip-light above mirror – slightly cooler
        glUniform3f(glGetUniformLocation(wash_sh, "lightPos2"), WASH_X + 2.5f, 3.5f, WASH_Z - 4.85f);
        glUniform3f(glGetUniformLocation(wash_sh, "lightColor2"), 0.95f, 0.97f, 1.00f);
        glUniform1f(glGetUniformLocation(wash_sh, "lightIntensity2"), 0.75f);
    }
    else {
        glUniform3f(glGetUniformLocation(wash_sh, "lightPos"), 0.f, 4.f, 0.f);
        glUniform3f(glGetUniformLocation(wash_sh, "lightColor"), 0.f, 0.f, 0.f);
        glUniform1f(glGetUniformLocation(wash_sh, "lightIntensity"), 0.f);
        glUniform3f(glGetUniformLocation(wash_sh, "lightPos2"), 0.f, 4.f, 0.f);
        glUniform3f(glGetUniformLocation(wash_sh, "lightColor2"), 0.f, 0.f, 0.f);
        glUniform1f(glGetUniformLocation(wash_sh, "lightIntensity2"), 0.f);
    }
}

// ── Bezier surface builder ───────────────────────────────────────────────────
static void buildBezierSurface(GLfloat* cp, int L, int nt, int ntheta,
    unsigned int& outVAO, int& outIdxCount)
{
    const double pi = 3.14159265358979;
    bezVerts.clear();
    bezIdxs.clear();

    int stride = ntheta + 1;
    for (int i = 0; i <= nt; ++i) {
        float t = (float)i / nt;
        double x = 0, y = 0;
        for (int k = 0; k <= L; ++k) {
            long long ncr = 1;
            for (int j = 1; j <= k; ++j) ncr = ncr * (L - k + j) / j;
            double b = pow(1.0 - t, L - k) * pow(t, k) * ncr;
            x += b * cp[k * 3 + 0];
            y += b * cp[k * 3 + 1];
        }
        float r = (float)x;
        float yy = (float)y;
        float li = (r > 0.01f) ? 1.f / r : 0.f;

        for (int j = 0; j <= ntheta; ++j) {
            double theta = 2.0 * pi * j / ntheta;
            float  xx = r * (float)cos(theta);
            float  zz = -r * (float)sin(theta);
            bezVerts.push_back(xx); bezVerts.push_back(yy); bezVerts.push_back(zz);
            bezVerts.push_back(xx * li); bezVerts.push_back(0.f); bezVerts.push_back(zz * li);
        }
    }

    for (int i = 0; i < nt; ++i)
        for (int j = 0; j < ntheta; ++j) {
            int a = i * stride + j, b = a + 1, c = a + stride, d = c + 1;
            bezIdxs.push_back(a); bezIdxs.push_back(b); bezIdxs.push_back(c);
            bezIdxs.push_back(b); bezIdxs.push_back(d); bezIdxs.push_back(c);
        }

    unsigned int vbo, ebo;
    glGenVertexArrays(1, &outVAO);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(outVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bezVerts.size() * sizeof(float), bezVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bezIdxs.size() * sizeof(unsigned int), bezIdxs.data(), GL_STATIC_DRAW);
    outIdxCount = (int)bezIdxs.size();
    glBindVertexArray(0);
}

// ── Bezier initialiser ───────────────────────────────────────────────────────
static void initWashroomBezier()
{
    if (washroomInitDone) return;
    washroomInitDone = true;

    // ── Toilet bowl – smooth S-curve, narrower at base, flared rim ──────────
    // Each row: x(radius), y(height), ignored-z
    GLfloat toiletBowlCP[] = {
        0.28f, 0.00f, 0.f,   // base ring
        0.30f, 0.06f, 0.f,   // slight flare at foot
        0.32f, 0.18f, 0.f,
        0.30f, 0.32f, 0.f,   // waist
        0.24f, 0.46f, 0.f,   // inner taper
        0.22f, 0.60f, 0.f,
        0.26f, 0.74f, 0.f,   // bowl expands
        0.30f, 0.86f, 0.f,
        0.34f, 0.95f, 0.f,   // rim
        0.36f, 1.00f, 0.f    // top edge
    };
    buildBezierSurface(toiletBowlCP, 9, 32, 32, toiletBowlVAO, toiletBowlIdx);

    // ── Toilet cistern – squat oval shape ───────────────────────────────────
    GLfloat toiletTankCP[] = {
        0.00f, 0.00f, 0.f,
        0.28f, 0.02f, 0.f,
        0.30f, 0.12f, 0.f,
        0.30f, 0.50f, 0.f,
        0.30f, 0.88f, 0.f,
        0.28f, 0.98f, 0.f,
        0.00f, 1.00f, 0.f
    };
    buildBezierSurface(toiletTankCP, 6, 20, 32, toiletTankVAO, toiletTankIdx);

    // ── Sink basin – elegant under-mount bowl ────────────────────────────────
    GLfloat sinkBasinCP[] = {
        0.00f, 0.00f, 0.f,   // bottom centre
        0.14f, 0.02f, 0.f,
        0.30f, 0.08f, 0.f,
        0.42f, 0.18f, 0.f,
        0.48f, 0.30f, 0.f,   // widest interior
        0.50f, 0.42f, 0.f,
        0.52f, 0.52f, 0.f,   // rim
        0.54f, 0.58f, 0.f,
        0.55f, 0.62f, 0.f
    };
    buildBezierSurface(sinkBasinCP, 8, 28, 40, sinkBasinVAO, sinkBasinIdx);

    // ── Sink pedestal – tapered column ──────────────────────────────────────
    GLfloat sinkPedCP[] = {
        0.22f, 0.00f, 0.f,   // wide foot
        0.22f, 0.05f, 0.f,
        0.20f, 0.12f, 0.f,
        0.18f, 0.30f, 0.f,   // taper
        0.16f, 0.60f, 0.f,
        0.14f, 0.80f, 0.f,
        0.15f, 0.92f, 0.f,
        0.16f, 1.00f, 0.f    // top joins basin underside
    };
    buildBezierSurface(sinkPedCP, 7, 20, 32, sinkPedestalVAO, sinkPedestalIdx);

    // ── Bathtub – freestanding oval tub profile ──────────────────────────────
    GLfloat bathtubCP[] = {
        0.00f, 0.00f, 0.f,   // base centre
        0.30f, 0.02f, 0.f,
        0.55f, 0.06f, 0.f,
        0.70f, 0.14f, 0.f,
        0.78f, 0.26f, 0.f,
        0.80f, 0.40f, 0.f,   // widest
        0.82f, 0.56f, 0.f,
        0.80f, 0.68f, 0.f,   // outer wall
        0.78f, 0.78f, 0.f,
        0.75f, 0.84f, 0.f,
        0.72f, 0.90f, 0.f,
        0.70f, 0.94f, 0.f,
        0.68f, 1.00f, 0.f    // rim
    };
    buildBezierSurface(bathtubCP, 12, 36, 48, bathtubVAO, bathtubIdx);

    // ── Round shower-head disc ───────────────────────────────────────────────
    GLfloat showerCP[] = {
        0.00f, 0.00f, 0.f,
        0.08f, 0.01f, 0.f,
        0.16f, 0.02f, 0.f,
        0.22f, 0.01f, 0.f,
        0.26f, -0.01f, 0.f,
        0.28f, -0.03f, 0.f,
        0.28f, -0.07f, 0.f
    };
    buildBezierSurface(showerCP, 6, 14, 36, showerHeadVAO, showerHeadIdx);

    // ── Faucet spout cross-section ───────────────────────────────────────────
    GLfloat faucetCP[] = {
        0.035f, 0.00f, 0.f,
        0.036f, 0.08f, 0.f,
        0.035f, 0.22f, 0.f,
        0.030f, 0.40f, 0.f,
        0.025f, 0.55f, 0.f,
        0.022f, 0.65f, 0.f
    };
    buildBezierSurface(faucetCP, 5, 12, 16, faucetVAO, faucetIdx);
}

// ── Draw a Bezier VAO with given transform and colour ────────────────────────
static void drawBez(unsigned int vao, int idxCount,
    glm::mat4 m, glm::vec3 col)
{
    if (!vao) return;
    glBindVertexArray(vao);
    setModel(m);
    setColor(col);
    noTexture();
    glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
}

// ═════════════════════════════════════════════════════════════════════════════
//  ROOM SHELL
// ═════════════════════════════════════════════════════════════════════════════
static void drawWashroomShell()
{
    const float wx = WASH_X, wz = WASH_Z;
    const float hw = WASH_W * 0.5f, hd = WASH_D * 0.5f;

    // ── Palette ──────────────────────────────────────────────────────────────
    const glm::vec3 floorCol(0.82f, 0.80f, 0.78f);  // warm light-grey marble
    const glm::vec3 wallCol(0.93f, 0.92f, 0.90f);  // soft off-white
    const glm::vec3 ceilCol(0.96f, 0.96f, 0.95f);
    const glm::vec3 groutCol(0.55f, 0.54f, 0.52f);  // darker for depth
    const glm::vec3 wainCol(0.88f, 0.87f, 0.85f);  // wainscoting
    const glm::vec3 accentCol(0.40f, 0.62f, 0.72f);  // steel-blue accent strip

    // Floor – marble tile texture, high-frequency tiling
    washBox(wx, 0.f, wz, WASH_W, 0.10f, WASH_D, floorCol, texTile, 3.f);
    // Ceiling
    washBox(wx, WASH_H, wz, WASH_W, 0.10f, WASH_D, ceilCol, texConcrete, 1.f);

    // Walls
    washBox(wx - hw, 2.f, wz, 0.12f, WASH_H, WASH_D, wallCol, texConcrete, 2.f);
    washBox(wx + hw, 2.f, wz, 0.12f, WASH_H, WASH_D, wallCol, texConcrete, 2.f);
    washBox(wx, 2.f, wz - hd, WASH_W, WASH_H, 0.12f, wallCol, texConcrete, 2.f);
    washBox(wx, 2.f, wz + hd, WASH_W, WASH_H, 0.12f, wallCol, texConcrete, 2.f);

    // ── Wainscoting / dado rail (lower 1 m of all walls) ────────────────────
    const float wainH = 1.10f;
    washBox(wx - hw + 0.06f, wainH * 0.5f, wz, 0.06f, wainH, WASH_D - 0.02f, wainCol, texTile, 2.f);
    washBox(wx + hw - 0.06f, wainH * 0.5f, wz, 0.06f, wainH, WASH_D - 0.02f, wainCol, texTile, 2.f);
    washBox(wx, wainH * 0.5f, wz - hd + 0.06f, WASH_W, wainH, 0.06f, wainCol, texTile, 2.f);
    washBox(wx, wainH * 0.5f, wz + hd - 0.06f, WASH_W, wainH, 0.06f, wainCol, texTile, 2.f);

    // Dado cap rail (thin horizontal strip)
    washBox(wx - hw + 0.06f, wainH, wz, 0.04f, 0.06f, WASH_D, accentCol);
    washBox(wx + hw - 0.06f, wainH, wz, 0.04f, 0.06f, WASH_D, accentCol);
    washBox(wx, wainH, wz - hd + 0.06f, WASH_W, 0.06f, 0.04f, accentCol);
    washBox(wx, wainH, wz + hd - 0.06f, WASH_W, 0.06f, 0.04f, accentCol);

    // ── Floor grout grid ─────────────────────────────────────────────────────
    //    300 × 300 mm tiles  →  grid every 0.6 world-units (scale 1u ≈ 0.5 m)
    const float tileStep = 0.60f;
    const float groutThk = 0.012f;
    const float groutRise = 0.006f;
    for (float z = wz - hd + tileStep; z < wz + hd; z += tileStep)
        washBox(wx, groutRise, z, WASH_W - 0.02f, groutThk * 0.8f, groutThk, groutCol);
    for (float x = wx - hw + tileStep; x < wx + hw; x += tileStep)
        washBox(x, groutRise, wz, groutThk, groutThk * 0.8f, WASH_D - 0.02f, groutCol);

    // ── Skirting board along floor/wall joint ────────────────────────────────
    const glm::vec3 skirtCol(0.75f, 0.74f, 0.72f);
    washBox(wx - hw + 0.06f, 0.075f, wz, 0.04f, 0.15f, WASH_D, skirtCol);
    washBox(wx + hw - 0.06f, 0.075f, wz, 0.04f, 0.15f, WASH_D, skirtCol);
    washBox(wx, 0.075f, wz - hd + 0.06f, WASH_W, 0.15f, 0.04f, skirtCol);
    washBox(wx, 0.075f, wz + hd - 0.06f, WASH_W, 0.15f, 0.04f, skirtCol);
}

// ═════════════════════════════════════════════════════════════════════════════
//  TOILET
// ═════════════════════════════════════════════════════════════════════════════
static void drawToilet()
{
    initWashroomBezier();

    const float tx = WASH_X - 2.8f;
    const float tz = WASH_Z - 3.8f;
    const glm::vec3 porcCol(0.96f, 0.95f, 0.93f);   // warm porcelain

    // ── Bowl ─────────────────────────────────────────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { tx, 0.36f, tz });
        m = glm::scale(m, { 0.72f, 0.60f, 0.72f });
        drawBez(toiletBowlVAO, toiletBowlIdx, m, porcCol);
    }
    // Seat ring (slightly darker, sits atop bowl rim)
    washBox(tx, 0.74f, tz, 0.76f, 0.04f, 0.82f, glm::vec3(0.88f, 0.87f, 0.84f));
    // Seat lid (white, slightly raised)
    washBox(tx, 0.80f, tz, 0.74f, 0.03f, 0.78f, glm::vec3(0.95f, 0.94f, 0.92f));

    // ── Cistern ──────────────────────────────────────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { tx, 0.80f, tz - 0.42f });
        m = glm::scale(m, { 0.72f, 0.58f, 0.36f });   // squished in Z = oval side-on
        drawBez(toiletTankVAO, toiletTankIdx, m, porcCol);
    }
    // Cistern lid
    washBox(tx, 1.20f, tz - 0.42f, 0.72f, 0.035f, 0.38f, glm::vec3(0.94f, 0.94f, 0.92f));
    // Cistern flush button
    washBox(tx, 1.24f, tz - 0.42f, 0.12f, 0.025f, 0.12f, glm::vec3(0.80f, 0.80f, 0.80f));

    // ── Floor connector (short trapezoid "foot") ─────────────────────────────
    washBox(tx, 0.18f, tz, 0.55f, 0.36f, 0.55f, porcCol);
    // Chrome flush pipe
    washBox(tx, 0.95f, tz - 0.28f, 0.04f, 0.18f, 0.04f, glm::vec3(0.78f, 0.78f, 0.76f));
}

// ═════════════════════════════════════════════════════════════════════════════
//  SINK
// ═════════════════════════════════════════════════════════════════════════════
static void drawSink()
{
    initWashroomBezier();

    const float sx = WASH_X + 2.5f;
    const float sz = WASH_Z - 3.8f;
    const glm::vec3 porcCol(0.96f, 0.95f, 0.93f);
    const glm::vec3 chromeCol(0.82f, 0.82f, 0.80f);

    // ── Pedestal ─────────────────────────────────────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { sx, 0.f, sz });
        m = glm::scale(m, { 0.80f, 0.84f, 0.80f });
        drawBez(sinkPedestalVAO, sinkPedestalIdx, m, porcCol);
    }

    // ── Basin (upside-down Bezier bowl, open top) ────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { sx, 0.84f, sz });
        m = glm::scale(m, { 1.10f, 0.38f, 1.10f });
        drawBez(sinkBasinVAO, sinkBasinIdx, m, porcCol);
    }
    // Rim highlight
    washBox(sx, 1.24f, sz, 1.25f, 0.04f, 1.25f, glm::vec3(0.92f, 0.91f, 0.89f));

    // ── Counter top surrounding basin ────────────────────────────────────────
    washBox(sx, 1.22f, sz, 1.80f, 0.06f, 1.80f,
        glm::vec3(0.90f, 0.88f, 0.86f), texTile, 1.f);

    // ── Faucet (Bezier spout + chrome handles) ───────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { sx, 1.28f, sz - 0.36f });
        m = glm::scale(m, { 1.0f, 1.0f, 1.0f });
        drawBez(faucetVAO, faucetIdx, m, chromeCol);
    }
    // Spout arc (horizontal bar + short down-pipe)
    washBox(sx, 1.55f, sz - 0.20f, 0.06f, 0.06f, 0.36f, chromeCol);
    washBox(sx, 1.44f, sz + 0.02f, 0.06f, 0.22f, 0.06f, chromeCol);
    // Handle knobs
    washBox(sx - 0.28f, 1.32f, sz - 0.38f, 0.16f, 0.06f, 0.06f, chromeCol);
    washBox(sx + 0.28f, 1.32f, sz - 0.38f, 0.16f, 0.06f, 0.06f, chromeCol);
    // Drain dot
    washBox(sx, 0.86f, sz, 0.06f, 0.03f, 0.06f, glm::vec3(0.50f, 0.50f, 0.48f));
}

// ═════════════════════════════════════════════════════════════════════════════
//  BATHTUB (freestanding oval)
// ═════════════════════════════════════════════════════════════════════════════
static void drawBathtub()
{
    initWashroomBezier();

    const float bx = WASH_X - 1.8f;
    const float bz = WASH_Z + 2.6f;
    const glm::vec3 tubCol(0.95f, 0.94f, 0.92f);  // warm white
    const glm::vec3 waterCol(0.62f, 0.78f, 0.88f);  // translucent water tint
    const glm::vec3 chromeCol(0.80f, 0.80f, 0.78f);

    // ── Main tub shell (Bezier revolution) ───────────────────────────────────
    {
        // Elongated by scaling X heavily
        glm::mat4 m = glm::translate(glm::mat4(1.f), { bx, 0.f, bz });
        m = glm::scale(m, { 2.40f, 0.70f, 1.20f });
        drawBez(bathtubVAO, bathtubIdx, m, tubCol);
    }

    // ── Water surface (flat rectangle inside the rim) ─────────────────────────
    washBox(bx, 0.65f, bz, 3.20f, 0.03f, 1.80f, waterCol);

    // ── Ball-and-claw feet (4 stumpy chrome spheroids) ───────────────────────
    for (float fx : {bx - 1.6f, bx + 1.6f})
        for (float fz : {bz - 0.85f, bz + 0.85f}) {
            washBox(fx, 0.0f, fz, 0.16f, 0.10f, 0.16f, chromeCol);
            washBox(fx, 0.1f, fz, 0.12f, 0.08f, 0.12f, glm::vec3(0.60f, 0.58f, 0.55f));
        }

    // ── Overflow cover + deck-mounted faucet ────────────────────────────────
    washBox(bx + 1.55f, 0.48f, bz, 0.04f, 0.20f, 0.20f, tubCol);        // overflow plate
    washBox(bx + 1.55f, 0.52f, bz, 0.06f, 0.04f, 0.06f, chromeCol);     // screw
    washBox(bx + 1.20f, 0.74f, bz - 0.10f, 0.06f, 0.30f, 0.06f, chromeCol); // spout riser
    washBox(bx + 1.20f, 1.00f, bz - 0.10f, 0.06f, 0.06f, 0.40f, chromeCol); // spout arm
    washBox(bx + 1.20f, 0.85f, bz + 0.28f, 0.06f, 0.28f, 0.06f, chromeCol); // down-spout
    // Handles
    washBox(bx + 1.20f, 0.82f, bz - 0.20f, 0.20f, 0.05f, 0.05f, chromeCol);
    washBox(bx + 1.20f, 0.82f, bz + 0.00f, 0.20f, 0.05f, 0.05f, chromeCol);
}

// ═════════════════════════════════════════════════════════════════════════════
//  SHOWER CUBICLE
// ═════════════════════════════════════════════════════════════════════════════
static void drawShower()
{
    initWashroomBezier();

    const float shx = WASH_X + 2.2f;
    const float shz = WASH_Z + 3.0f;
    const glm::vec3 glassCol(0.80f, 0.88f, 0.92f);  // frosted glass
    const glm::vec3 chromeCol(0.78f, 0.78f, 0.76f);
    const glm::vec3 tileCol(0.86f, 0.85f, 0.82f);

    // ── Shower tray (raised kerb) ────────────────────────────────────────────
    washBox(shx, 0.07f, shz, 2.40f, 0.14f, 2.40f, glm::vec3(0.82f, 0.80f, 0.78f), texTile, 1.f);
    // Drain
    washBox(shx, 0.15f, shz, 0.18f, 0.03f, 0.18f, glm::vec3(0.45f, 0.45f, 0.43f));
    // Drain grate lines
    for (float d = -0.06f; d <= 0.07f; d += 0.04f)
        washBox(shx + d, 0.17f, shz, 0.02f, 0.015f, 0.16f, glm::vec3(0.35f, 0.35f, 0.33f));

    // ── Glass panels (two sides, chrome frame) ───────────────────────────────
    // Side panel (along Z-axis wall)
    washBox(shx - 1.2f, 1.2f, shz, 0.04f, 2.4f, 2.40f, glassCol);
    washBox(shx - 1.2f, 1.2f, shz, 0.06f, 2.4f, 0.04f, chromeCol); // top rail
    // Front panel
    washBox(shx, 1.2f, shz - 1.2f, 2.40f, 2.4f, 0.04f, glassCol);
    // Chrome door frame
    washBox(shx - 1.2f, 2.4f, shz - 1.2f, 2.40f, 0.06f, 2.40f, chromeCol); // top rail
    washBox(shx - 1.2f, 1.2f, shz - 1.2f, 0.06f, 2.4f, 0.06f, chromeCol);  // corner post
    // Door handle bar
    washBox(shx - 0.8f, 1.4f, shz - 1.22f, 0.06f, 0.8f, 0.06f, chromeCol);

    // ── Shower head (Bezier disc) ────────────────────────────────────────────
    {
        glm::mat4 m = glm::translate(glm::mat4(1.f), { shx - 0.8f, 2.50f, shz });
        // rotate so disc faces downward
        m = glm::rotate(m, glm::radians(90.f), { 1.f, 0.f, 0.f });
        m = glm::scale(m, { 1.0f, 1.0f, 1.0f });
        drawBez(showerHeadVAO, showerHeadIdx, m, chromeCol);
    }
    // Arm from wall to head
    washBox(shx - 0.8f, 2.62f, shz - 0.55f, 0.06f, 0.06f, 1.00f, chromeCol);
    // Wall elbow
    washBox(shx - 0.8f, 2.62f, shz - 1.0f, 0.10f, 0.16f, 0.10f, chromeCol);

    // ── Wall tiles inside cubicle ─────────────────────────────────────────────
    //    Two rear walls tiled to waist-height extra (full height here)
    washBox(shx - 1.18f, 1.5f, shz, 0.03f, 3.0f, 2.38f, tileCol, texTile, 2.f);
    washBox(shx, 1.5f, shz + 1.18f, 2.38f, 3.0f, 0.03f, tileCol, texTile, 2.f);
}

// ═════════════════════════════════════════════════════════════════════════════
//  VANITY MIRROR
// ═════════════════════════════════════════════════════════════════════════════
static void drawMirror()
{
    const float mx = WASH_X + 2.5f;
    const float mz = WASH_Z - 4.84f;

    const glm::vec3 frameCol(0.72f, 0.70f, 0.68f);   // brushed nickel
    const glm::vec3 glassCol(0.78f, 0.84f, 0.88f);   // mirror tint
    const glm::vec3 ledCol(1.00f, 0.99f, 0.95f);   // LED strip

    // ── Mirror glass ─────────────────────────────────────────────────────────
    washBox(mx, 2.80f, mz + 0.05f, 2.20f, 1.90f, 0.04f, glassCol);

    // ── Slim bevelled frame ───────────────────────────────────────────────────
    washBox(mx, 2.80f, mz, 2.40f, 2.10f, 0.08f, frameCol);  // back plate
    washBox(mx, 3.86f, mz + 0.04f, 2.40f, 0.12f, 0.12f, frameCol);  // top rail
    washBox(mx, 1.74f, mz + 0.04f, 2.40f, 0.12f, 0.12f, frameCol);  // bottom rail
    washBox(mx - 1.2f, 2.80f, mz + 0.04f, 0.12f, 2.10f, 0.12f, frameCol); // left
    washBox(mx + 1.2f, 2.80f, mz + 0.04f, 0.12f, 2.10f, 0.12f, frameCol); // right

    // ── LED vanity strip above mirror ─────────────────────────────────────────
    if (washLightOn) setEmissive(ledCol);
    washBox(mx, 3.95f, mz + 0.04f, 2.00f, 0.08f, 0.06f, ledCol);
    setEmissive(glm::vec3(0.f));

    // ── Medicine cabinet shelf (below mirror) ─────────────────────────────────
    washBox(mx, 1.68f, mz + 0.10f, 2.30f, 0.06f, 0.18f, frameCol);
    // Small toiletry items on shelf
    washBox(mx - 0.6f, 1.76f, mz + 0.10f, 0.12f, 0.22f, 0.12f, glm::vec3(0.85f, 0.90f, 0.85f)); // bottle
    washBox(mx + 0.1f, 1.76f, mz + 0.10f, 0.09f, 0.18f, 0.09f, glm::vec3(0.90f, 0.85f, 0.82f)); // bottle
    washBox(mx + 0.6f, 1.76f, mz + 0.10f, 0.14f, 0.14f, 0.14f, glm::vec3(0.82f, 0.86f, 0.90f)); // jar
}

// ═════════════════════════════════════════════════════════════════════════════
//  TOWEL WARMER RACK (heated rail)
// ═════════════════════════════════════════════════════════════════════════════
static void drawTowelRack()
{
    const float rx = WASH_X - 3.80f;
    const float rz = WASH_Z + 1.0f;

    const glm::vec3 metalCol(0.72f, 0.72f, 0.70f);
    const glm::vec3 towelCol1(0.94f, 0.88f, 0.80f);  // cream
    const glm::vec3 towelCol2(0.75f, 0.82f, 0.88f);  // powder-blue

    // Five horizontal rails
    float rails[] = { 1.30f, 1.60f, 1.90f, 2.20f, 2.50f };
    for (float rh : rails)
        washBox(rx + 0.04f, rh, rz, 0.04f, 0.04f, 1.20f, metalCol);

    // Two vertical uprights
    washBox(rx + 0.04f, 1.90f, rz - 0.55f, 0.04f, 1.24f, 0.04f, metalCol);
    washBox(rx + 0.04f, 1.90f, rz + 0.55f, 0.04f, 1.24f, 0.04f, metalCol);

    // Wall bracket plates
    washBox(rx + 0.02f, 1.28f, rz - 0.55f, 0.06f, 0.08f, 0.14f, metalCol);
    washBox(rx + 0.02f, 1.28f, rz + 0.55f, 0.06f, 0.08f, 0.14f, metalCol);
    washBox(rx + 0.02f, 2.52f, rz - 0.55f, 0.06f, 0.08f, 0.14f, metalCol);
    washBox(rx + 0.02f, 2.52f, rz + 0.55f, 0.06f, 0.08f, 0.14f, metalCol);

    // Two draped towels
    washBox(rx + 0.05f, 1.95f, rz - 0.10f, 0.06f, 0.55f, 0.90f, towelCol1);
    washBox(rx + 0.05f, 1.70f, rz + 0.20f, 0.06f, 0.45f, 0.70f, towelCol2);
}


// ═════════════════════════════════════════════════════════════════════════════
//  CEILING LIGHT PANEL
// ═════════════════════════════════════════════════════════════════════════════
static void drawLightFixture()
{
    const glm::vec3 ledWhite(0.95f, 0.96f, 0.98f);
    const glm::vec3 frameCol(0.70f, 0.70f, 0.68f);

    // Recessed panel frame
    washBox(WASH_X, WASH_H - 0.02f, WASH_Z, 1.80f, 0.04f, 1.80f, frameCol);

    if (washLightOn) setEmissive(ledWhite);
    // LED diffuser panel
    washBox(WASH_X, WASH_H - 0.05f, WASH_Z, 1.70f, 0.03f, 1.70f, ledWhite);
    setEmissive(glm::vec3(0.f));
}

// ═════════════════════════════════════════════════════════════════════════════
//  ACCESSORIES
// ═════════════════════════════════════════════════════════════════════════════
static void drawAccessories()
{
    const glm::vec3 chromCol(0.76f, 0.76f, 0.74f);
    const glm::vec3 plasCol(0.86f, 0.86f, 0.84f);
    const glm::vec3 soapCol(0.90f, 0.94f, 0.90f);  // mint soap dispenser

    // ── Toilet roll holder ───────────────────────────────────────────────────
    washBox(WASH_X - 3.72f, 1.10f, WASH_Z - 2.4f, 0.04f, 0.04f, 0.40f, chromCol);
    washBox(WASH_X - 3.72f, 1.10f, WASH_Z - 2.2f, 0.04f, 0.04f, 0.04f, chromCol);
    washBox(WASH_X - 3.72f, 1.10f, WASH_Z - 2.4f, 0.04f, 0.04f, 0.04f, chromCol);
    washBox(WASH_X - 3.72f, 1.10f, WASH_Z - 2.3f, 0.14f, 0.22f, 0.24f, glm::vec3(0.95f, 0.94f, 0.92f));

    // ── Soap dispenser beside sink ───────────────────────────────────────────
    washBox(WASH_X + 1.6f, 1.34f, WASH_Z - 3.8f, 0.16f, 0.30f, 0.16f, soapCol);
    washBox(WASH_X + 1.6f, 1.50f, WASH_Z - 3.8f, 0.06f, 0.14f, 0.06f, chromCol); // pump

    // ── Toothbrush holder ────────────────────────────────────────────────────
    washBox(WASH_X + 3.5f, 1.34f, WASH_Z - 3.5f, 0.22f, 0.28f, 0.22f, plasCol);
    // Toothbrushes
    washBox(WASH_X + 3.44f, 1.56f, WASH_Z - 3.5f, 0.04f, 0.36f, 0.04f, glm::vec3(0.85f, 0.42f, 0.42f));
    washBox(WASH_X + 3.56f, 1.56f, WASH_Z - 3.5f, 0.04f, 0.36f, 0.04f, glm::vec3(0.42f, 0.55f, 0.85f));

    // ── Small rubber bath mat ────────────────────────────────────────────────
    washBox(WASH_X + 2.5f, 0.06f, WASH_Z - 2.8f, 1.20f, 0.04f, 0.60f, glm::vec3(0.40f, 0.55f, 0.52f));

    // ── Waste bin ────────────────────────────────────────────────────────────
    washBox(WASH_X + 3.6f, 0.28f, WASH_Z - 1.8f, 0.34f, 0.56f, 0.34f, plasCol);
    washBox(WASH_X + 3.6f, 0.56f, WASH_Z - 1.8f, 0.32f, 0.04f, 0.32f, chromCol);  // rim

    // ── Small potted plant (corner life) ────────────────────────────────────
    washBox(WASH_X - 3.5f, 0.30f, WASH_Z + 4.4f, 0.36f, 0.60f, 0.36f, glm::vec3(0.55f, 0.42f, 0.32f));
    washBox(WASH_X - 3.5f, 0.80f, WASH_Z + 4.4f, 0.14f, 0.04f, 0.14f, glm::vec3(0.35f, 0.28f, 0.20f));
    washBox(WASH_X - 3.5f, 0.90f, WASH_Z + 4.4f, 0.40f, 0.30f, 0.40f, glm::vec3(0.30f, 0.55f, 0.30f));
    washBox(WASH_X - 3.5f, 1.06f, WASH_Z + 4.3f, 0.28f, 0.28f, 0.28f, glm::vec3(0.28f, 0.58f, 0.28f));
    washBox(WASH_X - 3.3f, 0.98f, WASH_Z + 4.55f, 0.28f, 0.22f, 0.28f, glm::vec3(0.26f, 0.52f, 0.26f));
}

// ═════════════════════════════════════════════════════════════════════════════
//  PUBLIC ENTRY POINTS
// ═════════════════════════════════════════════════════════════════════════════
void drawWashroom(unsigned int sh)
{
    wash_sh = sh;

    washLighting();
    drawWashroomShell();
    drawToilet();
    drawSink();
    drawBathtub();
    drawShower();
    drawMirror();
    drawTowelRack();
    drawLightFixture();
    drawAccessories();
}

void updateWashroomFan()
{
    washFanAngle += 1.5f;
    if (washFanAngle >= 360.f) washFanAngle -= 360.f;
}

void toggleWashroomLight()
{
    washLightOn = !washLightOn;
}
