#include "utils.h"
#include "globals.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>

// ── Camera vectors ────────────────────────────────────────────────────────────
void updateCameraVectors() {
    glm::vec3 front;
    front.x = cosf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));
    front.y = sinf(glm::radians(camera.pitch));
    front.z = sinf(glm::radians(camera.yaw)) * cosf(glm::radians(camera.pitch));
    camera.front = glm::normalize(front);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

// ── Custom perspective ────────────────────────────────────────────────────────
glm::mat4 customPerspective(float fovY, float aspect, float zN, float zF) {
    float t = tanf(fovY / 2.f);
    glm::mat4 r(0.f);
    r[0][0] = 1.f / (aspect * t);
    r[1][1] = 1.f / t;
    r[2][2] = -(zF + zN) / (zF - zN);
    r[2][3] = -1.f;
    r[3][2] = -(2.f * zF * zN) / (zF - zN);
    return r;
}

// ── Framebuffer resize callback ───────────────────────────────────────────────
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    SCR_WIDTH = w; SCR_HEIGHT = h;
    glViewport(0, 0, w, h);
}

// ── Integer to string ─────────────────────────────────────────────────────────
std::string itos(int v) {
    std::ostringstream ss; ss << v; return ss.str();
}

// ── Lighting uniforms ─────────────────────────────────────────────────────────
void setupLighting(unsigned int sh) {
    glUniform1i(glGetUniformLocation(sh, "directionalLightOn"), lighting.directionalLightOn);
    glUniform1i(glGetUniformLocation(sh, "pointLightsOn"), lighting.pointLightsOn);
    glUniform1i(glGetUniformLocation(sh, "spotLightOn"), lighting.spotLightOn);
    glUniform1i(glGetUniformLocation(sh, "ambientOn"), lighting.ambientOn);
    glUniform1i(glGetUniformLocation(sh, "diffuseOn"), lighting.diffuseOn);
    glUniform1i(glGetUniformLocation(sh, "specularOn"), lighting.specularOn);
    glUniform3f(glGetUniformLocation(sh, "viewPos"),
        camera.position.x, camera.position.y, camera.position.z);

    if (dayMode) {
        glUniform3f(glGetUniformLocation(sh, "dirLight_direction"), -0.5f, -1.0f, -0.4f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_ambient"), 0.30f, 0.30f, 0.28f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_diffuse"), 0.80f, 0.78f, 0.70f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_specular"), 1.00f, 1.00f, 0.90f);
    }
    else {
        glUniform3f(glGetUniformLocation(sh, "dirLight_direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_ambient"), 0.05f, 0.05f, 0.10f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_diffuse"), 0.10f, 0.10f, 0.20f);
        glUniform3f(glGetUniformLocation(sh, "dirLight_specular"), 0.30f, 0.30f, 0.50f);
    }

    glm::vec3 ptPos[] = {
        {-15.f, 9.f,  8.f},
        { 10.f, 9.f,  8.f},
        { -5.f,14.f,-18.f},
        { -5.f, 9.f,-30.f},
        { -5.f, 2.8f, 3.f},
        {  8.f, 2.8f, 3.f}
    };
    glm::vec3 ptCol[] = {
        {1.f,0.9f,0.7f},{0.9f,0.9f,1.f},
        {1.f,0.9f,0.8f},{0.8f,1.f,0.9f},
        {1.f,0.97f,0.85f},{1.f,0.97f,0.85f}
    };
    float ptConst[] = { 1.f,1.f,1.f,1.f,1.f,1.f };
    float ptLinear[] = { 0.027f,0.027f,0.027f,0.027f,0.14f,0.14f };
    float ptQuad[] = { 0.0028f,0.0028f,0.0028f,0.0028f,0.07f,0.07f };

    for (int i = 0; i < 6; i++) {
        std::string n = itos(i);
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_position[" + n + "]").c_str()), 1, glm::value_ptr(ptPos[i]));
        glUniform3f(glGetUniformLocation(sh, ("pointLights_ambient[" + n + "]").c_str()),
            (i >= 4) ? 0.35f : 0.05f, (i >= 4) ? 0.35f : 0.05f, (i >= 4) ? 0.32f : 0.05f);
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_diffuse[" + n + "]").c_str()), 1, glm::value_ptr(ptCol[i] * 0.7f));
        glUniform3fv(glGetUniformLocation(sh, ("pointLights_specular[" + n + "]").c_str()), 1, glm::value_ptr(ptCol[i]));
        glUniform1f(glGetUniformLocation(sh, ("pointLights_constant[" + n + "]").c_str()), ptConst[i]);
        glUniform1f(glGetUniformLocation(sh, ("pointLights_linear[" + n + "]").c_str()), ptLinear[i]);
        glUniform1f(glGetUniformLocation(sh, ("pointLights_quadratic[" + n + "]").c_str()), ptQuad[i]);
    }

    glUniform3f(glGetUniformLocation(sh, "spotLight_position"),
        camera.position.x, camera.position.y, camera.position.z);
    glUniform3f(glGetUniformLocation(sh, "spotLight_direction"),
        camera.front.x, camera.front.y, camera.front.z);
    glUniform1f(glGetUniformLocation(sh, "spotLight_cutOff"), cosf(glm::radians(12.5f)));
    glUniform3f(glGetUniformLocation(sh, "spotLight_ambient"), 0, 0, 0);
    glUniform3f(glGetUniformLocation(sh, "spotLight_diffuse"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(sh, "spotLight_specular"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(sh, "emissive"), 0, 0, 0);
}

// ── Print instructions ────────────────────────────────────────────────────────
void printInstructions() {
    std::cout << "\n+==================================================================+" << std::endl;
    std::cout << "|       STUDENT DORMITORY - KUET  (Full Campus Simulation)        |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  TEXTURE MODES (T to cycle):                                    |" << std::endl;
    std::cout << "|   0 - No Texture       : Phong lighting x objectColor only      |" << std::endl;
    std::cout << "|   1 - Simple Texture   : Phong x texColor                       |" << std::endl;
    std::cout << "|   2 - Vertex Gouraud   : Gouraud (vertex) blended 50/50 w/ tex  |" << std::endl;
    std::cout << "|   3 - Fragment Phong   : Phong (fragment) blended 50/50 w/ tex  |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  LIGHTING TOGGLES:                                              |" << std::endl;
    std::cout << "|   1/2/3 - Dir/Point/Spot  light ON/OFF                          |" << std::endl;
    std::cout << "|   5/6/7 - Ambient/Diffuse/Specular ON/OFF                       |" << std::endl;
    std::cout << "|   L     - Day / Night toggle                                    |" << std::endl;
    std::cout << "+==================================================================+" << std::endl;
    std::cout << "|  CAMERA:  W/S/A/D fwd/back/left/right   E/R up/down             |" << std::endl;
    std::cout << "|           X/Y/Z pitch/yaw/roll   F orbit   I bird-eye   P reset |" << std::endl;
    std::cout << "|  VIEWPORT: V+0 split  V+1 iso  V+2 front  V+3 top  V+4 free     |" << std::endl;
    std::cout << "|  ESC - Quit                                                     |" << std::endl;
    std::cout << "+==================================================================+\n" << std::endl;

    std::cout << "|   Q      - Move Down (was R — R now opens dorm room)            |" << std::endl;
    std::cout << "|   R      - Enter dorm room (Room 101, first-person)             |" << std::endl;
    std::cout << "|   B      - Return to campus exterior                            |" << std::endl;

}