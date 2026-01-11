#pragma once
#include <glad/glad.h>

class CubeMesh
{
public:
    CubeMesh();
    ~CubeMesh();

    void Draw() const;

private:
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
};

