#pragma once

// ── Cricket layout constants (used by pitch, players, and ball) ───────────────
static const float CR_X = 58.f;
static const float CR_Z = 4.f;
static const float CR_Y = 0.06f;
static const float CR_PITCH_L = 20.12f;
static const float CR_PITCH_W = 3.05f;
static const float CR_OVAL_R = 18.f;
static const float CR_BOWL_Z = CR_Z - CR_PITCH_L * 0.47f;
static const float CR_BAT_Z = CR_Z + CR_PITCH_L * 0.47f;
static const float CR_CYCLE = 3.2f;

// ── Public render functions ───────────────────────────────────────────────────
void renderCricketPitch(unsigned int sh);
void renderBatsman(unsigned int sh);
void renderCricketBall(unsigned int sh);