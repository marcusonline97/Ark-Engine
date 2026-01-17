#include "Cube.h"
#include <GLAD/glad.h>

CubeMesh::CubeMesh()
{
    // pos + normal (36 verts)
    const float vertices[] = {
        // -Z
        -0.5f, -0.5f, -0.5f,  0, 0, -1,
         0.5f, -0.5f, -0.5f,  0, 0, -1,
         0.5f,  0.5f, -0.5f,  0, 0, -1,
         0.5f,  0.5f, -0.5f,  0, 0, -1,
        -0.5f,  0.5f, -0.5f,  0, 0, -1,
        -0.5f, -0.5f, -0.5f,  0, 0, -1,

        // +Z
        -0.5f, -0.5f,  0.5f,  0, 0,  1,
         0.5f, -0.5f,  0.5f,  0, 0,  1,
         0.5f,  0.5f,  0.5f,  0, 0,  1,
         0.5f,  0.5f,  0.5f,  0, 0,  1,
        -0.5f,  0.5f,  0.5f,  0, 0,  1,
        -0.5f, -0.5f,  0.5f,  0, 0,  1,

        // -X
        -0.5f,  0.5f,  0.5f, -1, 0,  0,
        -0.5f,  0.5f, -0.5f, -1, 0,  0,
        -0.5f, -0.5f, -0.5f, -1, 0,  0,
        -0.5f, -0.5f, -0.5f, -1, 0,  0,
        -0.5f, -0.5f,  0.5f, -1, 0,  0,
        -0.5f,  0.5f,  0.5f, -1, 0,  0,

        // +X
         0.5f,  0.5f,  0.5f,  1, 0,  0,
         0.5f,  0.5f, -0.5f,  1, 0,  0,
         0.5f, -0.5f, -0.5f,  1, 0,  0,
         0.5f, -0.5f, -0.5f,  1, 0,  0,
         0.5f, -0.5f,  0.5f,  1, 0,  0,
         0.5f,  0.5f,  0.5f,  1, 0,  0,

         // -Y
         -0.5f, -0.5f, -0.5f,  0,-1,  0,
          0.5f, -0.5f, -0.5f,  0,-1,  0,
          0.5f, -0.5f,  0.5f,  0,-1,  0,
          0.5f, -0.5f,  0.5f,  0,-1,  0,
         -0.5f, -0.5f,  0.5f,  0,-1,  0,
         -0.5f, -0.5f, -0.5f,  0,-1,  0,

         // +Y
         -0.5f,  0.5f, -0.5f,  0, 1,  0,
          0.5f,  0.5f, -0.5f,  0, 1,  0,
          0.5f,  0.5f,  0.5f,  0, 1,  0,
          0.5f,  0.5f,  0.5f,  0, 1,  0,
         -0.5f,  0.5f,  0.5f,  0, 1,  0,
         -0.5f,  0.5f, -0.5f,  0, 1,  0,
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

CubeMesh::~CubeMesh()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void CubeMesh::Draw() const
{
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}