#include "Infinite_Grid_Technique.h"


bool InfiniteGridTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Resources/Shaders/infinite_grid.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Resources/Shaders/infinite_grid.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    return InitCommon();
}


bool InfiniteGridTechnique::InitCommon()
{
    GET_UNIFORM_AND_CHECK(VPLoc, "gVP");
    GET_UNIFORM_AND_CHECK(CameraWorldPosLoc, "gCameraWorldPos");
    GET_UNIFORM_AND_CHECK(GridCellSizeLoc, "gGridCellSize");

    return true;
}


void InfiniteGridTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void InfiniteGridTechnique::SetCameraWorldPos(const Vector3f& CameraWorldPos)
{
    glUniform3f(CameraWorldPosLoc, CameraWorldPos.x, CameraWorldPos.y, CameraWorldPos.z);
}


void InfiniteGridTechnique::SetCellSize(float CellSize)
{
    glUniform1f(GridCellSizeLoc, CellSize);
}