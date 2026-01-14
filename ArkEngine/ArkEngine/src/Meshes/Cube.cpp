#include "Cube.h"

CubeMesh::CubeMesh()
{
    float vertices[] = {
        // positions           // colors              // uvs
        // back face (-Z)
        -0.5f, -0.5f, -0.5f,   1,0,0,               0,0,
         0.5f, -0.5f, -0.5f,   0,1,0,               1,0,
         0.5f,  0.5f, -0.5f,   0,0,1,               1,1,
         0.5f,  0.5f, -0.5f,   0,0,1,               1,1,
        -0.5f,  0.5f, -0.5f,   1,1,0,               0,1,
        -0.5f, -0.5f, -0.5f,   1,0,0,               0,0,

        // front face (+Z)
        -0.5f, -0.5f,  0.5f,   1,0,1,               0,0,
         0.5f, -0.5f,  0.5f,   0,1,1,               1,0,
         0.5f,  0.5f,  0.5f,   1,1,1,               1,1,
         0.5f,  0.5f,  0.5f,   1,1,1,               1,1,
        -0.5f,  0.5f,  0.5f,   0.5f,0.5f,0.5f,     0,1,
        -0.5f, -0.5f,  0.5f,   1,0,1,               0,0,

        // left face (-X)
        -0.5f,  0.5f,  0.5f,   0.2f,0.8f,0.2f,     1,0,
        -0.5f,  0.5f, -0.5f,   0.2f,0.2f,0.8f,     1,1,
        -0.5f, -0.5f, -0.5f,   0.8f,0.2f,0.2f,     0,1,
        -0.5f, -0.5f, -0.5f,   0.8f,0.2f,0.2f,     0,1,
        -0.5f, -0.5f,  0.5f,   0.8f,0.8f,0.2f,     0,0,
        -0.5f,  0.5f,  0.5f,   0.2f,0.8f,0.2f,     1,0,

        // right face (+X)
         0.5f,  0.5f,  0.5f,   0.2f,0.8f,0.8f,     1,0,
         0.5f,  0.5f, -0.5f,   0.8f,0.2f,0.8f,     1,1,
         0.5f, -0.5f, -0.5f,   0.8f,0.8f,0.8f,     0,1,
         0.5f, -0.5f, -0.5f,   0.8f,0.8f,0.8f,     0,1,
         0.5f, -0.5f,  0.5f,   0.2f,0.2f,0.2f,     0,0,
         0.5f,  0.5f,  0.5f,   0.2f,0.8f,0.8f,     1,0,

        // bottom face (-Y)
        -0.5f, -0.5f, -0.5f,   0.9f,0.3f,0.3f,     0,1,
         0.5f, -0.5f, -0.5f,   0.3f,0.9f,0.3f,     1,1,
         0.5f, -0.5f,  0.5f,   0.3f,0.3f,0.9f,     1,0,
         0.5f, -0.5f,  0.5f,   0.3f,0.3f,0.9f,     1,0,
        -0.5f, -0.5f,  0.5f,   0.9f,0.9f,0.3f,     0,0,
        -0.5f, -0.5f, -0.5f,   0.9f,0.3f,0.3f,     0,1,

        // top face (+Y)
        -0.5f,  0.5f, -0.5f,   0.3f,0.9f,0.9f,     0,1,
         0.5f,  0.5f, -0.5f,   0.9f,0.3f,0.9f,     1,1,
         0.5f,  0.5f,  0.5f,   0.9f,0.9f,0.9f,     1,0,
         0.5f,  0.5f,  0.5f,   0.9f,0.9f,0.9f,     1,0,
        -0.5f,  0.5f,  0.5f,   0.3f,0.3f,0.3f,     0,0,
        -0.5f,  0.5f, -0.5f,   0.3f,0.9f,0.9f,     0,1
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
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
