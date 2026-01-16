#pragma once

#include "Math/3DMath_util.h"
#include "Utility/Util.h"
#include "Utility/Technique.h"


class InfiniteGridTechnique : public Technique
{
public:
    InfiniteGridTechnique() {}

    virtual bool Init();

    void SetVP(const Matrix4f& VP);
    void SetCameraWorldPos(const Vector3f& CameraWorldPos);
    void SetCellSize(float CellSize);

private:
    bool InitCommon();

    GLuint VPLoc = INVALID_UNIFORM_LOCATION;
    GLuint CameraWorldPosLoc = INVALID_UNIFORM_LOCATION;
    GLuint GridCellSizeLoc = INVALID_UNIFORM_LOCATION;
};