#include "Utility/Technique.h"
#include "Math/3DMath_util.h"
#include "Rendering/Lightning/Lightning.h"


class SkinningTechnique : public LightingTechnique
{
public:

    SkinningTechnique();

    virtual bool Init();

    void SetBoneTransform(uint Index, const Matrix4f& Transform);

private:

    GLuint m_boneLocation[MAX_BONES];
};