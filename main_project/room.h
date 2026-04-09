#pragma once

// ── Bedroom ───────────────────────────────────────────────────────────────────
void initRoom();                  // call once at startup (caches shader handle)
void drawBedroom();               // draw one frame of the bedroom
void updateBedroomFan();          // advance fan angle (call each frame)
void toggleBedroomLight();        // toggle ceiling light on/off

extern float brFanAngle;
extern bool  brLightOn;

// ── Reading Room ──────────────────────────────────────────────────────────────
void drawReadingRoom();           // draw one frame of the reading room
void updateReadingRoomFan();      // advance fan angle
void toggleReadingRoomLight();    // toggle ceiling light on/off

extern float rrFanAngle;
extern bool  rrLightOn;

// ── Dispatcher (used by renderScene) ──────────────────────────────────────────
void renderRoom(unsigned int shader);