#include "curves.h"
#include "globals.h"
#include "draw_helpers.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

// ─── Bezier object handles ───────────────────────────────────────────────────
BezObj bezDome;
BezObj bezArch;
BezObj bezPedestal;
BezObj bezPillar;
BezObj bezRailing;

// Shared geometry vectors (cleared before each bez_hollowBezier call)
static std::vector<float> bez_coordinates;
static std::vector<float> bez_normals;
static std::vector<int>   bez_indices;
static std::vector<float> bez_vertices;

// ══════════════════════════════════════════════════════════════════════════════
//  BEZIER MATH HELPERS
// ══════════════════════════════════════════════════════════════════════════════
long long bez_nCr(int n, int r) {
    if (r > n / 2) r = n - r;
    long long ans = 1;
    for (int i = 1; i <= r; i++) { ans *= n - r + i; ans /= i; }
    return ans;
}

void bez_BezierCurve(double t, float xy[2], GLfloat ctrlpoints[], int L) {
    double y = 0, x = 0;
    t = t > 1.0 ? 1.0 : t;
    for (int i = 0; i <= L; i++) {
        long long ncr = bez_nCr(L, i);
        double oneMinusTpow = pow(1 - t, double(L - i));
        double tPow = pow(t, double(i));
        double coef = oneMinusTpow * tPow * ncr;
        x += coef * ctrlpoints[i * 3];
        y += coef * ctrlpoints[(i * 3) + 1];
    }
    xy[0] = float(x);
    xy[1] = float(y);
}

// ══════════════════════════════════════════════════════════════════════════════
//  HOLLOW BEZIER (surface of revolution)
// ══════════════════════════════════════════════════════════════════════════════
unsigned int bez_hollowBezier(GLfloat ctrlpoints[], int L,
    int nt, int ntheta,
    int& outIndexCount)
{
    bez_coordinates.clear();
    bez_normals.clear();
    bez_indices.clear();
    bez_vertices.clear();

    const double pi = 3.14159265389;
    const double dtheta = 2.0 * pi / ntheta;
    float  t = 0;
    float  dt = 1.0f / nt;
    float  xy[2];

    for (int i = 0; i <= nt; ++i) {
        bez_BezierCurve(t, xy, ctrlpoints, L);
        float r = xy[0];
        float y = xy[1];
        float theta = 0;
        float lengthInv = (r != 0.f) ? 1.0f / r : 0.f;
        t += dt;

        for (int j = 0; j <= ntheta; ++j) {
            double cosa = cos(theta);
            double sina = sin(theta);
            float  z = -r * float(sina);
            float  x = r * float(cosa);

            bez_coordinates.push_back(x);
            bez_coordinates.push_back(y);
            bez_coordinates.push_back(z);

            float nx = (x - 0) * lengthInv;
            float ny = 0;
            float nz = (z - 0) * lengthInv;

            bez_normals.push_back(nx);
            bez_normals.push_back(ny);
            bez_normals.push_back(nz);

            theta += float(dtheta);
        }
    }

    int k1, k2;
    for (int i = 0; i < nt; ++i) {
        k1 = i * (ntheta + 1);
        k2 = k1 + ntheta + 1;
        for (int j = 0; j < ntheta; ++j, ++k1, ++k2) {
            bez_indices.push_back(k1);
            bez_indices.push_back(k2);
            bez_indices.push_back(k1 + 1);
            bez_indices.push_back(k1 + 1);
            bez_indices.push_back(k2);
            bez_indices.push_back(k2 + 1);
        }
    }

    size_t count = bez_coordinates.size();
    for (size_t i = 0; i < count; i += 3) {
        bez_vertices.push_back(bez_coordinates[i]);
        bez_vertices.push_back(bez_coordinates[i + 1]);
        bez_vertices.push_back(bez_coordinates[i + 2]);
        bez_vertices.push_back(bez_normals[i]);
        bez_vertices.push_back(bez_normals[i + 1]);
        bez_vertices.push_back(bez_normals[i + 2]);
        bez_vertices.push_back(bez_coordinates[i] * 0.5f + 0.5f);
        bez_vertices.push_back(bez_coordinates[i + 1] * 0.5f + 0.5f);
    }

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        bez_vertices.size() * sizeof(float),
        bez_vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        bez_indices.size() * sizeof(int),
        bez_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    outIndexCount = (int)bez_indices.size();
    return vao;
}

// ══════════════════════════════════════════════════════════════════════════════
//  DRAW HELPER FOR BEZIER OBJECTS
// ══════════════════════════════════════════════════════════════════════════════
static void drawBezObj(unsigned int sh, const BezObj& obj,
    glm::mat4 model, glm::vec3 color,
    unsigned int texID = 0, float tile = 1.f)
{
    if (!obj.vao) return;
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(sh, "uvTile"), tile);
    int effMode = (texID != 0) ? textureMode : 0;
    glUniform1i(glGetUniformLocation(sh, "texMode"), effMode);
    if (texID != 0 && textureMode != 0) {
        glUniform1i(glGetUniformLocation(sh, "hasTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        glUniform1i(glGetUniformLocation(sh, "texSampler"), 0);
    }
    else {
        glUniform1i(glGetUniformLocation(sh, "hasTexture"), 0);
    }
    glBindVertexArray(obj.vao);
    glDrawElements(GL_TRIANGLES, obj.indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ══════════════════════════════════════════════════════════════════════════════
//  SETUP BEZIER OBJECTS
// ══════════════════════════════════════════════════════════════════════════════
void setupBezierObjects() {
    // 1. DOME
    {
        GLfloat cp[] = {
            0.00f,1.00f,0.0f,  0.20f,0.90f,0.0f,
            0.90f,0.40f,0.0f,  1.00f,0.00f,0.0f
        };
        bezDome.vao = bez_hollowBezier(cp, 3, 30, 36, bezDome.indexCount);
    }
    // 2. ARCH RING
    {
        GLfloat cp[] = {
            0.55f, 0.00f,0.0f, 0.55f,0.30f,0.0f, 0.75f,0.60f,0.0f,
            0.75f, 1.00f,0.0f, 0.60f,1.00f,0.0f
        };
        bezArch.vao = bez_hollowBezier(cp, 4, 20, 36, bezArch.indexCount);
    }
    // 3. DECORATIVE VASE / PEDESTAL
    {
        GLfloat cp[] = {
            0.20f,0.00f,0.0f, 0.28f,0.10f,0.0f, 0.45f,0.35f,0.0f,
            0.50f,0.50f,0.0f, 0.42f,0.65f,0.0f, 0.22f,0.78f,0.0f,
            0.22f,0.88f,0.0f, 0.38f,0.95f,0.0f, 0.40f,1.00f,0.0f
        };
        bezPedestal.vao = bez_hollowBezier(cp, 8, 40, 32, bezPedestal.indexCount);
    }
    // 4. ENTASIS PILLAR
    {
        GLfloat cp[] = {
            0.50f,0.00f,0.0f, 0.52f,0.20f,0.0f, 0.52f,0.40f,0.0f,
            0.46f,0.70f,0.0f, 0.38f,1.00f,0.0f
        };
        bezPillar.vao = bez_hollowBezier(cp, 4, 28, 28, bezPillar.indexCount);
    }
    // 5. TURNED RAILING POST (spindle)
    {
        GLfloat cp[] = {
            0.12f,0.00f,0.0f, 0.14f,0.08f,0.0f, 0.08f,0.20f,0.0f,
            0.06f,0.45f,0.0f, 0.06f,0.55f,0.0f, 0.10f,0.70f,0.0f,
            0.14f,0.78f,0.0f, 0.12f,0.88f,0.0f, 0.12f,1.00f,0.0f
        };
        bezRailing.vao = bez_hollowBezier(cp, 8, 30, 20, bezRailing.indexCount);
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  RENDER BEZIER OBJECTS
// ══════════════════════════════════════════════════════════════════════════════
void renderBezierObjects(unsigned int sh) {
    glm::mat4 m;

    // 1. DOME on Block A roof (removed)

    // 2. ARCH RING over the gate
    m = glm::translate(glm::mat4(1.f), { -5.f, 8.1f, 35.f });
    m = glm::scale(m, { 6.f, 0.8f, 1.2f });
    drawBezObj(sh, bezArch, m, COL_COLUMN, texConcrete, 1.f);

    // 3. DECORATIVE VASE (×2, flanking courtyard fountain)
    for (int s : {-1, 1}) {
        float vx = -4.f + s * 4.f;
        m = glm::translate(glm::mat4(1.f), { vx, 0.3f, -12.f });
        m = glm::scale(m, { 0.7f, 1.6f, 0.7f });
        drawBezObj(sh, bezPedestal, m, COL_MARBLE, texMarble, 1.f);
    }

    // 4. ENTASIS PILLARS (×4, admin annex entrance)
    for (int i = 0; i < 4; ++i) {
        float px = 16.f + i * 3.f;
        m = glm::translate(glm::mat4(1.f), { px, 0.f, -3.5f });
        m = glm::scale(m, { 1.0f, 12.f, 1.0f });
        drawBezObj(sh, bezPillar, m, COL_COLUMN, texConcrete, 2.f);
    }

    // 5. TURNED RAILING POSTS along Block-A front balcony (floors 1-4)
    {
        const float FLOOR_H = 3.2f;
        const float AX = -5.f;
        const float ALEN = 40.f;
        const float ZN_POST = 12.5f;

        for (int fl = 1; fl <= 4; ++fl) {
            float fBase = fl * FLOOR_H;
            for (int ri = 0; ri <= 10; ++ri) {
                float rx = AX - ALEN * 0.5f + ri * (ALEN / 10.f);
                m = glm::translate(glm::mat4(1.f), { rx, fBase + 0.12f, ZN_POST });
                m = glm::scale(m, { 0.16f, 1.3f, 0.16f });
                drawBezObj(sh, bezRailing, m, COL_RAILING, 0, 1.f);
            }
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//  SPLINE RAILING  (stub — original code forward-declared drawSplineRailing
//  but never defined it in the provided source; a no-op stub is safe)
// ══════════════════════════════════════════════════════════════════════════════
void setupSplineRailing() {
    // Intentionally empty — no geometry was defined in the original source.
    // If you add spline geometry later, initialise splineVAO/splineVBO here.
}

void drawSplineRailing(unsigned int /*sh*/) {
    if (!splineVAO || splineVertCount == 0) return;
    // Draw call would go here once geometry is added.
}

// ══════════════════════════════════════════════════════════════════════════════
//  RULED SURFACE  —  utility + gate roof
// ══════════════════════════════════════════════════════════════════════════════
void generateRuledSurface(const std::vector<glm::vec3>& C0pts,
    const std::vector<glm::vec3>& C1pts,
    int NU, int NV,
    std::vector<float>& verts,
    std::vector<unsigned int>& idxs)
{
    verts.clear();
    idxs.clear();

    auto samplePolyline = [&](const std::vector<glm::vec3>& pts, float u) -> glm::vec3 {
        float scaled = u * (float)(pts.size() - 1);
        int   lo = (int)scaled;
        float frac = scaled - (float)lo;
        lo = std::min(lo, (int)pts.size() - 2);
        return (1.f - frac) * pts[lo] + frac * pts[lo + 1];
        };

    std::vector<std::vector<glm::vec3>> grid(NU + 1, std::vector<glm::vec3>(NV + 1));
    for (int i = 0; i <= NU; ++i) {
        float u = (float)i / (float)NU;
        glm::vec3 p0 = samplePolyline(C0pts, u);
        glm::vec3 p1 = samplePolyline(C1pts, u);
        for (int j = 0; j <= NV; ++j) {
            float v = (float)j / (float)NV;
            grid[i][j] = (1.f - v) * p0 + v * p1;
        }
    }

    auto getNormal = [&](int i, int j) -> glm::vec3 {
        int ip = std::min(i + 1, NU), im = std::max(i - 1, 0);
        int jp = std::min(j + 1, NV), jm = std::max(j - 1, 0);
        glm::vec3 du = grid[ip][j] - grid[im][j];
        glm::vec3 dv = grid[i][jp] - grid[i][jm];
        glm::vec3 n = glm::cross(du, dv);
        float len = glm::length(n);
        return (len > 1e-6f) ? n / len : glm::vec3(0.f, 1.f, 0.f);
        };

    for (int i = 0; i <= NU; ++i)
        for (int j = 0; j <= NV; ++j) {
            glm::vec3 p = grid[i][j];
            glm::vec3 n = getNormal(i, j);
            float u = (float)i / NU;
            float v = (float)j / NV;
            verts.insert(verts.end(), { p.x,p.y,p.z, n.x,n.y,n.z, u,v });
        }

    auto idx = [&](int i, int j) -> unsigned int {
        return (unsigned int)(i * (NV + 1) + j);
        };
    for (int i = 0; i < NU; ++i)
        for (int j = 0; j < NV; ++j) {
            unsigned int a = idx(i, j), b = idx(i + 1, j), c = idx(i + 1, j + 1), d = idx(i, j + 1);
            idxs.insert(idxs.end(), { a,b,c, a,c,d });
            idxs.insert(idxs.end(), { a,c,b, a,d,c });
        }
}

void setupRuledSurfaceRoof() {
    const float GX = -5.f;
    const float GZ = 35.f;
    const float LINTY = 8.1f;
    const float HALF = 6.2f;

    std::vector<glm::vec3> C0 = {
        {GX - HALF,        LINTY + 0.0f, GZ},
        {GX - HALF * 0.5f,   LINTY + 1.8f, GZ},
        {GX,             LINTY + 2.5f, GZ},
        {GX + HALF * 0.5f,   LINTY + 1.8f, GZ},
        {GX + HALF,        LINTY + 0.0f, GZ}
    };
    const float OVERHANG = 0.8f;
    std::vector<glm::vec3> C1 = {
        {GX - HALF - OVERHANG, LINTY, GZ},
        {GX - HALF * 0.5f,     LINTY, GZ},
        {GX,               LINTY, GZ},
        {GX + HALF * 0.5f,     LINTY, GZ},
        {GX + HALF + OVERHANG, LINTY, GZ}
    };

    std::vector<float>        verts;
    std::vector<unsigned int> idxs;
    generateRuledSurface(C0, C1, 30, 12, verts, idxs);

    glGenVertexArrays(1, &ruledVAO);
    glGenBuffers(1, &ruledVBO);
    glGenBuffers(1, &ruledEBO);

    glBindVertexArray(ruledVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ruledVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ruledEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxs.size() * sizeof(unsigned int), idxs.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    ruledIndexCount = (int)idxs.size();
}

void drawRuledSurfaceRoof(unsigned int sh) {
    if (!ruledVAO) return;
    glm::mat4 identity(1.f);
    glUniformMatrix4fv(glGetUniformLocation(sh, "model"), 1, GL_FALSE, glm::value_ptr(identity));
    glm::vec3 roofColor(0.70f, 0.72f, 0.74f);
    glUniform3fv(glGetUniformLocation(sh, "objectColor"), 1, glm::value_ptr(roofColor));
    glUniform1f(glGetUniformLocation(sh, "uvTile"), 2.f);
    int effMode = (texConcrete != 0) ? textureMode : 0;
    glUniform1i(glGetUniformLocation(sh, "texMode"), effMode);
    if (texConcrete != 0 && textureMode != 0) {
        glUniform1i(glGetUniformLocation(sh, "hasTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texConcrete);
        glUniform1i(glGetUniformLocation(sh, "texSampler"), 0);
    }
    else {
        glUniform1i(glGetUniformLocation(sh, "hasTexture"), 0);
    }
    glBindVertexArray(ruledVAO);
    glDrawElements(GL_TRIANGLES, ruledIndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}