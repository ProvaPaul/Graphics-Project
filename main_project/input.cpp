#include "input.h"
#include "globals.h"   // has Camera, inRoomView, savedCameraPos, etc.
#include "utils.h"     // has updateCameraVectors()
#include "room.h"      // has inRoomView extern (already in globals.h but included for clarity)
#include "corridor.h"
#include <iostream>

extern int currentRoom;

// NOTE: removed #include "camera.h" — your project has no camera.h
//       Camera struct and all camera variables live in globals.h

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float vel = camera.speed * deltaTime;

    // ── Movement (works both inside and outside room) ──────────────────────
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.position += camera.front * vel;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.position -= camera.front * vel;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.position -= camera.right * vel;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.position += camera.right * vel;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.position += camera.up * vel;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.position -= camera.up * vel;

    // ── Room boundary collision (clamp camera within room walls) ────────────
    if (inRoomView && currentRoom != 7) {
        float roomMinX, roomMaxX, roomMinZ, roomMaxZ;

        if (currentRoom == 6) {
            roomMinX = -14.0f; roomMaxX = 14.0f;
            roomMinZ = -18.0f; roomMaxZ = 5.0f;
        }
        else if (currentRoom == 8) {
            // Washroom shell is centered around X=5, Z=18 with size ~8x10
            roomMinX = 1.2f; roomMaxX = 8.8f;
            roomMinZ = 13.2f; roomMaxZ = 22.8f;
        }
        else {
            roomMinX = -9.5f; roomMaxX = 9.5f;
            roomMinZ = -14.5f; roomMaxZ = 4.8f;
        }

        camera.position.x = glm::clamp(camera.position.x, roomMinX, roomMaxX);
        camera.position.z = glm::clamp(camera.position.z, roomMinZ, roomMaxZ);
        camera.position.y = 1.8f;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { camera.pitch += 1.f; updateCameraVectors(); }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) { camera.yaw += 1.f; updateCameraVectors(); }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { camera.roll += 1.f; updateCameraVectors(); }

    // ── P  reset camera (campus) / return to corridor (room) ─────────────
    static bool pP = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pP) {
        if (inRoomView && currentRoom != 7) {
            currentRoom = 7;
            initCorridor();

            std::cout << "\n";
            std::cout << "╔═══════════════════════════════════════════════════╗\n";
            std::cout << "║         RETURNED TO CORRIDOR                     ║\n";
            std::cout << "╚═══════════════════════════════════════════════════╝\n\n";
        }
        else if (!inRoomView) {
            camera.position = glm::vec3(0.f, 38.f, 170.f);
            camera.yaw = -90.f; camera.pitch = -16.f; camera.roll = 0.f;
            updateCameraVectors();
            std::cout << "[P] Camera reset to overview." << std::endl;
        }
        pP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pP = false;

    // ── F  orbit mode ──────────────────────────────────────────────────────
    static bool fP = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fP) {
        if (!inRoomView) {
            camera.orbitMode = !camera.orbitMode;
            std::cout << "Orbit: " << (camera.orbitMode ? "ON" : "OFF") << std::endl;
        }
        fP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fP = false;

    // ── I  bird's eye mode ─────────────────────────────────────────────────
    static bool iP = false;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iP) {
        if (!inRoomView) {
            camera.birdsEyeMode = !camera.birdsEyeMode;
            std::cout << "Bird's Eye: " << (camera.birdsEyeMode ? "ON" : "OFF") << std::endl;
        }
        iP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) iP = false;

    // ── T  cycle texture mode ──────────────────────────────────────────────
    static bool tP = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tP) {
        textureMode = (textureMode + 1) % 4;
        tP = true;
        const char* names[] = {
            "0 No Texture", "1 Simple Texture",
            "2 Vertex Gouraud Blend", "3 Fragment Phong Blend"
        };
        std::cout << "[T] Texture Mode -> " << names[textureMode] << std::endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tP = false;

    // ── Lighting toggles ───────────────────────────────────────────────────
#define TOGGLE(key,var,name) \
    { static bool p=false; \
      if (glfwGetKey(window,key)==GLFW_PRESS&&!p){var=!var;p=true;\
          std::cout<<name<<": "<<(var?"ON":"OFF")<<std::endl;} \
      else if (glfwGetKey(window,key)==GLFW_RELEASE) p=false; }

    TOGGLE(GLFW_KEY_1, lighting.directionalLightOn, "Dir Light")
        TOGGLE(GLFW_KEY_2, lighting.pointLightsOn, "Point Lights")
        TOGGLE(GLFW_KEY_3, lighting.spotLightOn, "Spot Light")
        TOGGLE(GLFW_KEY_5, lighting.ambientOn, "Ambient")
        TOGGLE(GLFW_KEY_6, lighting.diffuseOn, "Diffuse")
        TOGGLE(GLFW_KEY_7, lighting.specularOn, "Specular")
        TOGGLE(GLFW_KEY_L, dayMode, "Day/Night")
#undef TOGGLE

        // ── V+0..4  viewport switching ─────────────────────────────────────────
        static bool v0P = false, v1P = false, v2P = false, v3P = false, v4P = false;
    bool vHeld = (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
    if (vHeld && glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !v0P)
    {
        activeViewport = -1; v0P = true; std::cout << "[V+0] 4-Viewport\n";
    }
    else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE) v0P = false;

    if (vHeld && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !v1P)
    {
        activeViewport = 0; v1P = true; std::cout << "[V+1] Isometric\n";
    }
    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) v1P = false;

    if (vHeld && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !v2P)
    {
        activeViewport = 1; v2P = true; std::cout << "[V+2] Front View\n";
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) v2P = false;

    if (vHeld && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !v3P)
    {
        activeViewport = 2; v3P = true; std::cout << "[V+3] Top-Down\n";
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) v3P = false;

    if (vHeld && glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !v4P)
    {
        activeViewport = 3; v4P = true; std::cout << "[V+4] Free Camera\n";
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) v4P = false;

    // ══════════════════════════════════════════════════════════════════════
    //  R  →  Enter dorm room
    //  B  →  Return to campus
    // ══════════════════════════════════════════════════════════════════════
    static bool rP = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rP) {
        if (!inRoomView) {
            savedCameraPos = camera.position;
            savedYaw = camera.yaw;
            savedPitch = camera.pitch;

            currentRoom = 7;
            initCorridor();

            inRoomView = true;
            activeViewport = 3;

            std::cout << "\n";
            std::cout << "╔═══════════════════════════════════════════════════╗\n";
            std::cout << "║         ENTERED DORMITORY CORRIDOR               ║\n";
            std::cout << "╠═══════════════════════════════════════════════════╣\n";
            std::cout << "║  You are now in the main corridor.               ║\n";
            std::cout << "║  Walk up to a door and press E to enter.         ║\n";
            std::cout << "╠═══════════════════════════════════════════════════╣\n";
            std::cout << "║  W/S/A/D  →  walk around corridor               ║\n";
            std::cout << "║  E        →  enter selected door                 ║\n";
            std::cout << "║  P        →  return from room to corridor        ║\n";
            std::cout << "║  B        →  EXIT back to campus                 ║\n";
            std::cout << "║  L        →  toggle corridor light               ║\n";
            std::cout << "╚═══════════════════════════════════════════════════╝\n\n";
        }
        rP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rP = false;

    static bool eP = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !eP) {
        if (inRoomView && currentRoom == 7) {
            int roomToEnter = getSelectedDoorRoom();
            if (roomToEnter >= 0 && roomToEnter <= 8) {
                currentRoom = roomToEnter;

                if (roomToEnter == 8) {
                    camera.position = glm::vec3(5.0f, 1.80f, 14.0f);
                    camera.yaw = 180.0f; camera.pitch = 0.f; camera.roll = 0.f;
                } else {
                    camera.position = glm::vec3(0.0f, 1.80f, 3.5f);
                    camera.yaw = -90.0f; camera.pitch = 0.f; camera.roll = 0.f;
                }
                camera.orbitMode = false;
                camera.birdsEyeMode = false;
                updateCameraVectors();

                const char* roomNames[] = {"BEDROOM 1", "BEDROOM 2", "BEDROOM 3", "BEDROOM 4",
                    "DINING HALL", "READING ROOM", "TV LOUNGE", "CORRIDOR", "WASHROOM"};
                printf("\n[ENTERED] %s\n", roomNames[currentRoom]);
                printf("Press P to return to corridor.\n\n");
                fflush(stdout);
            }
        }
        eP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) eP = false;

    static bool bP = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bP) {
        if (inRoomView && currentRoom == 7) {
            // Always return to the default campus viewport camera
            // so exterior starts from the standard non-zoomed position.
            camera.position = glm::vec3(0.f, 38.f, 170.f);
            camera.yaw = -90.f;
            camera.pitch = -16.f;
            camera.roll = 0.f;
            camera.orbitMode = false;
            camera.birdsEyeMode = false;
            updateCameraVectors();

            inRoomView = false;
            activeViewport = -1;

            std::cout << "\n";
            std::cout << "╔═══════════════════════════════════════════════════╗\n";
            std::cout << "║         RETURNED TO CAMPUS EXTERIOR              ║\n";
            std::cout << "╚═══════════════════════════════════════════════════╝\n\n";
        }
        bP = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) bP = false;
}