#include "technique.h"
#include "Ark_math_3d.h"


class ShadowMappingTechnique : public Technique
{
public:

    ShadowMappingTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);

private:

    GLuint m_WVPLoc = INVALID_UNIFORM_LOCATION;
};
