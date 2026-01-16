#include "Utility/Technique.h"
#include "Math/3DMath_util.h"


class SkyboxTechnique : public Technique {
public:

    SkyboxTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetTextureUnit(unsigned int TextureUnit);

private:

    GLuint m_WVPLoc;
    GLuint m_textureLoc;
};
