
#include "InfiniteGrid.h"


InfiniteGrid::InfiniteGrid()
{

}


void InfiniteGrid::Init()
{
    if (!m_infiniteGridTech.Init()) {
        printf("Error initializing the infinite grid technique\n");
        exit(1);
    }
}


void InfiniteGrid::Render(const InfiniteGridConfig& Config, const Matrix4f& VP, const Vector3f& CameraPos)
{
    GLint CurProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &CurProg);

    m_infiniteGridTech.Enable();

    m_infiniteGridTech.SetVP(VP);
    m_infiniteGridTech.SetCameraWorldPos(CameraPos);
    m_infiniteGridTech.SetCellSize(Config.CellSize);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glDisable(GL_BLEND);

    glUseProgram(CurProg);
}