#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// ─── BezObj: one revolving Bezier surface ─────────────────────────────────────
struct BezObj {
    unsigned int vao = 0;
    int          indexCount = 0;
};

// Extern handles for the five named Bezier objects
extern BezObj bezDome;
extern BezObj bezArch;
extern BezObj bezPedestal;
extern BezObj bezPillar;
extern BezObj bezRailing;

// ── Low-level helpers (kept accessible for potential reuse) ───────────────────
long long bez_nCr(int n, int r);
void      bez_BezierCurve(double t, float xy[2], GLfloat ctrlpoints[], int L);

// Build a surface of revolution from a Bezier profile curve.
// Returns the VAO; outIndexCount receives the element count.
unsigned int bez_hollowBezier(GLfloat ctrlpoints[], int L,
    int nt, int ntheta,
    int& outIndexCount);

// ── Setup + render ────────────────────────────────────────────────────────────
void setupBezierObjects();
void renderBezierObjects(unsigned int sh);

// ── Spline railing ────────────────────────────────────────────────────────────
void setupSplineRailing();
void drawSplineRailing(unsigned int sh);

// ── Ruled surface roof ────────────────────────────────────────────────────────
// Core generation function (reusable)
void generateRuledSurface(const std::vector<glm::vec3>& C0pts,
    const std::vector<glm::vec3>& C1pts,
    int NU, int NV,
    std::vector<float>& verts,
    std::vector<unsigned int>& idxs);

void setupRuledSurfaceRoof();
void drawRuledSurfaceRoof(unsigned int sh);