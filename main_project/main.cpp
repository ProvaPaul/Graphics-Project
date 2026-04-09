#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "globals.h"
#include "shaders.h"
#include "textures.h"
#include "geometry.h"
#include "curves.h"
#include "utils.h"
#include "input.h"
#include "scene.h"
#include "room.h"
#include "tvroom.h"
#include "corridor.h"
#include "washroom.h"

// ── Forward declarations ──────────────────────────────────────────────────────
void drawBedroom();
void drawReadingRoom();
void updateBedroomFan();
void updateReadingRoomFan();
void toggleBedroomLight();
void toggleReadingRoomLight();

void drawDiningHall();
void updateDiningHallFan();
void toggleDiningHallLight();

void drawTVRoom(unsigned int sh);

void initCorridor();
void updateCorridor();
void drawCorridor();
void updateCorridorFan();
void toggleCorridorLight();

// ── Room selector ─────────────────────────────────────────────────────────────
// 0-3 = Bedrooms | 4 = Dining | 5 = Reading | 6 = TV | 7 = Corridor | 8 = Washroom
int currentRoom = 0;
int lastBedroomDoor = 0;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    SCR_WIDTH = mode->width; SCR_HEIGHT = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Student Dormitory - KUET Full Campus", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    SCR_WIDTH = fbW; SCR_HEIGHT = fbH;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    unsigned int shader = createShaderProgram();
    setupCube(); setupCylinder(32); setupSphere(32, 32); setupCone(32);
    setupBezierObjects();
    setupSplineRailing();
    setupRuledSurfaceRoof();
    createAllTextures();
    updateCameraVectors();
    printInstructions();
    initRoom();

    printf("\n=== ROOM CONTROLS ===\n");
    printf("  TAB  — Cycle rooms: Bedroom > Reading Room > Dining Hall > TV Room\n");
    printf("  L    — Toggle room light on/off\n");
    printf("  R    — Enter room view (from campus)\n");
    printf("  E    — Enter selected room from corridor\n");
    printf("  P    — Return to corridor from a room\n");
    printf("  B    — Return to campus from corridor\n");
    printf("  ESC  — Exit application\n");
    printf("  WASD / Mouse — Move & look around inside room\n");
    printf("=====================\n\n");
    fflush(stdout);

    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);

    const glm::vec3 SC(0.f, 8.f, -12.f);

    while (!glfwWindowShouldClose(window)) {
        float cf = (float)glfwGetTime();
        deltaTime = cf - lastFrame; lastFrame = cf;
        processInput(window);

        // ── TAB: cycle rooms (single-press guard) ─────────────────────────
        static bool tabWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if (!tabWasPressed) {
                if (currentRoom != 7) {
                    currentRoom = (currentRoom + 1) % 9;
                    tabWasPressed = true;
                    switch (currentRoom) {
                    case 0: printf("[ROOM] BEDROOM 1\n");     break;
                    case 1: printf("[ROOM] BEDROOM 2\n");     break;
                    case 2: printf("[ROOM] BEDROOM 3\n");     break;
                    case 3: printf("[ROOM] BEDROOM 4\n");     break;
                    case 4: printf("[ROOM] DINING HALL\n");   break;
                    case 5: printf("[ROOM] READING ROOM\n");  break;
                    case 6: printf("[ROOM] TV LOUNGE\n");     break;
                    case 7: printf("[ROOM] CORRIDOR\n");      break;
                    case 8: printf("[ROOM] WASHROOM\n");      break;
                    }
                    fflush(stdout);
                }
            }
        }
        else {
            tabWasPressed = false;
        }

        // ── L: toggle light for current room ──────────────────────────────
        static bool lWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            if (!lWasPressed) {
                switch (currentRoom) {
                case 0:
                case 1:
                case 2:
                case 3: toggleBedroomLight();    break;
                case 4: toggleDiningHallLight();  break;
                case 5: toggleReadingRoomLight(); break;
                case 6: /* TV room: add toggle here */ break;
                case 7: toggleCorridorLight();   break;
                case 8: toggleWashroomLight();   break;
                }
                lWasPressed = true;
                printf("[ROOM] Light toggled\n"); fflush(stdout);
            }
        }
        else {
            lWasPressed = false;
        }

        // ── Fan / animation update for current room ────────────────────────
        switch (currentRoom) {
        case 0:
        case 1:
        case 2:
        case 3: updateBedroomFan();     break;
        case 4: updateDiningHallFan();  break;
        case 5: updateReadingRoomFan(); break;
        case 6: /* TV room has no fan */ break;
        case 7: updateCorridorFan(); updateCorridor(); break;
        case 8: updateWashroomFan(); break;
        }

        // ── Clear colour ───────────────────────────────────────────────────
        if (inRoomView) {
            glClearColor(0.05f, 0.05f, 0.05f, 1.f);
        }
        else {
            glClearColor(
                dayMode ? 0.53f : 0.05f,
                dayMode ? 0.81f : 0.05f,
                dayMode ? 0.92f : 0.12f, 1.f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);
        setupLighting(shader);

            // ── ROOM VIEW ──────────────────────────────────────────────────────
        if (inRoomView) {
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

            // Dining Hall and TV Room get wider FOV; others use 60°
            float fov = (currentRoom == 4 || currentRoom == 6) ? 75.f : 60.f;
            float farP = (currentRoom == 4 || currentRoom == 6) ? 80.f : 50.f;

            glm::mat4 proj = glm::perspective(
                glm::radians(fov),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.05f, farP);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
                1, GL_FALSE, glm::value_ptr(proj));

            glm::vec3 rPos = camera.position;
            glm::vec3 rFront = camera.front;
            glm::mat4 view = glm::lookAt(rPos, rPos + rFront, camera.up);
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
                1, GL_FALSE, glm::value_ptr(view));
            glUniform3fv(glGetUniformLocation(shader, "viewPos"),
                1, glm::value_ptr(rPos));

            switch (currentRoom) {
            case 0:
            case 1:
            case 2:
            case 3: drawBedroom();          break;
            case 4: drawDiningHall();        break;
            case 5: drawReadingRoom();       break;
            case 6: drawTVRoom(shader);     break;
            case 7: drawCorridor();         break;
            case 8: drawWashroom(shader);    break;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // ── CAMPUS VIEW ────────────────────────────────────────────────────
        if (activeViewport == -1) {
            glm::mat4 proj = customPerspective(glm::radians(45.f),
                (float)(SCR_WIDTH / 2) / (float)(SCR_HEIGHT / 2), 0.1f, 500.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
                1, GL_FALSE, glm::value_ptr(proj));

            glViewport(0, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(60, 45, 60), SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            glViewport(SCR_WIDTH / 2, SCR_HEIGHT / 2, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(0, 12, 90), SC, { 0,1,0 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            {
                glm::mat4 v = glm::lookAt(SC + glm::vec3(0, 120, 0), SC, { 0,0,-1 });
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }

            glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);
            {
                glm::mat4 v;
                if (camera.birdsEyeMode)
                    v = glm::lookAt(SC + glm::vec3(0, 90, 0), SC, { 0,0,-1 });
                else if (camera.orbitMode) {
                    float ox = SC.x + sinf(cf) * 75.f, oz = SC.z + cosf(cf) * 75.f;
                    v = glm::lookAt({ ox, SC.y + 22.f, oz }, SC, { 0,1,0 });
                }
                else {
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
                }
                glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
                renderScene(shader);
            }
        }
        else {
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glm::mat4 proj = customPerspective(glm::radians(45.f),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
                1, GL_FALSE, glm::value_ptr(proj));

            glm::mat4 v;
            if (activeViewport == 0)
                v = glm::lookAt(SC + glm::vec3(60, 45, 60), SC, { 0,1,0 });
            else if (activeViewport == 1)
                v = glm::lookAt(SC + glm::vec3(0, 12, 90), SC, { 0,1,0 });
            else if (activeViewport == 2)
                v = glm::lookAt(SC + glm::vec3(0, 120, 0), SC, { 0,0,-1 });
            else {
                if (camera.birdsEyeMode)
                    v = glm::lookAt(SC + glm::vec3(0, 90, 0), SC, { 0,0,-1 });
                else if (camera.orbitMode) {
                    float ox = SC.x + sinf(cf) * 75.f, oz = SC.z + cosf(cf) * 75.f;
                    v = glm::lookAt({ ox, SC.y + 22.f, oz }, SC, { 0,1,0 });
                }
                else {
                    v = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
                }
            }
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
                1, GL_FALSE, glm::value_ptr(v));
            renderScene(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}