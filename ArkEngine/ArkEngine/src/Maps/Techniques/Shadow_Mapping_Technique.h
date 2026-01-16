#include "Utility/Technique.h"
#include "Math/3DMath_util.h"


class ShadowMappingTechnique : public Technique
{
public:

    ShadowMappingTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);

private:

    GLuint m_WVPLoc = INVALID_UNIFORM_LOCATION;
};