#include "technique.h"
#include "Ark_math_3d.h"
#include "Ark_new_lighting.h"


class SkinningTechnique : public LightingTechnique
{
public:

    SkinningTechnique();

    virtual bool Init();

    void SetBoneTransform(uint Index, const Matrix4f& Transform);

private:

    GLuint m_boneLocation[MAX_BONES];
};
