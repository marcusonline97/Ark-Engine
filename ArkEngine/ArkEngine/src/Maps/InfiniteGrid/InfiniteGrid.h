#pragma once


#include "Maps/Techniques/Infinite_Grid_Technique.h"

struct InfiniteGridConfig {
    float Size = 100.0f;
    float CellSize = 0.025f;
    Vector4f ColorThin = Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
    Vector4f ColorThick = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
    float MinPixelsBetweenCells = 2.0f;
};


class InfiniteGrid
{
public:
    InfiniteGrid();

    void Init();

    void Render(const InfiniteGridConfig& Config, const Matrix4f& VP, const Vector3f& CameraPos);

private:

    InfiniteGridTechnique m_infiniteGridTech;
}; 