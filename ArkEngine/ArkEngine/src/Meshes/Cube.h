#pragma once

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

