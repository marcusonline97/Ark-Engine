
#include "Ark_skinning_technique.h"



SkinningTechnique::SkinningTechnique()
{
}

bool SkinningTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/skinning.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/lighting_new.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    if (!InitCommon()) {
        return false;
    }

    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_boneLocation); i++) {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        SNPRINTF(Name, sizeof(Name), "gBones[%d]", i);
        m_boneLocation[i] = GetUniformLocation(Name);
    }

    return true;
}


void SkinningTechnique::SetBoneTransform(uint Index, const Matrix4f& Transform)
{
    //assert(Index < MAX_BONES);
    if (Index >= MAX_BONES) {
        return;
    }
    //Transform.Print();
    glUniformMatrix4fv(m_boneLocation[Index], 1, GL_TRUE, (const GLfloat*)Transform);
}