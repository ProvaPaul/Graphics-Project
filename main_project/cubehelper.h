#pragma once
#pragma once
// cube_helper.h
// Drop-in cube VAO that works with any GLFW + GLAD project.
// Call drawCube() after uploading your "model" uniform.
// No external dependencies beyond glad/glad.h.

#include <glad/glad.h>

namespace CubeHelper {

    static unsigned int _VAO = 0, _VBO = 0, _EBO = 0;

    // clang-format off
    static const float _verts[] = {
        // positions         // normals
        // +X face
         0.5f,-0.5f,-0.5f,  1,0,0,
         0.5f, 0.5f,-0.5f,  1,0,0,
         0.5f, 0.5f, 0.5f,  1,0,0,
         0.5f,-0.5f, 0.5f,  1,0,0,
         // -X face
         -0.5f,-0.5f, 0.5f, -1,0,0,
         -0.5f, 0.5f, 0.5f, -1,0,0,
         -0.5f, 0.5f,-0.5f, -1,0,0,
         -0.5f,-0.5f,-0.5f, -1,0,0,
         // +Y face
         -0.5f, 0.5f,-0.5f,  0,1,0,
          0.5f, 0.5f,-0.5f,  0,1,0,
          0.5f, 0.5f, 0.5f,  0,1,0,
         -0.5f, 0.5f, 0.5f,  0,1,0,
         // -Y face
         -0.5f,-0.5f, 0.5f,  0,-1,0,
          0.5f,-0.5f, 0.5f,  0,-1,0,
          0.5f,-0.5f,-0.5f,  0,-1,0,
         -0.5f,-0.5f,-0.5f,  0,-1,0,
         // +Z face
         -0.5f,-0.5f, 0.5f,  0,0,1,
          0.5f,-0.5f, 0.5f,  0,0,1,
          0.5f, 0.5f, 0.5f,  0,0,1,
         -0.5f, 0.5f, 0.5f,  0,0,1,
         // -Z face
          0.5f,-0.5f,-0.5f,  0,0,-1,
         -0.5f,-0.5f,-0.5f,  0,0,-1,
         -0.5f, 0.5f,-0.5f,  0,0,-1,
          0.5f, 0.5f,-0.5f,  0,0,-1,
    };
    static const unsigned int _idx[] = {
         0, 1, 2,  0, 2, 3,   // +X
         4, 5, 6,  4, 6, 7,   // -X
         8, 9,10,  8,10,11,   // +Y
        12,13,14, 12,14,15,   // -Y
        16,17,18, 16,18,19,   // +Z
        20,21,22, 20,22,23,   // -Z
    };
    // clang-format on

    inline void _init()
    {
        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);

        glBindVertexArray(_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(_verts), _verts, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_idx), _idx, GL_STATIC_DRAW);

        // position (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // normal (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

} // namespace CubeHelper

// Public free function — call this anywhere after initCubeHelper().
inline void initCubeHelper() { CubeHelper::_init(); }

inline void drawCube()
{
    if (CubeHelper::_VAO == 0) CubeHelper::_init();
    glBindVertexArray(CubeHelper::_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}