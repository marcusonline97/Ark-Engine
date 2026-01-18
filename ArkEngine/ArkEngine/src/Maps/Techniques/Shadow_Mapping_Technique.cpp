#include "Shadow_Mapping_Technique.h"

ShadowMappingTechnique::ShadowMappingTechnique()
{

}


bool ShadowMappingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "Resources/Shaders/shadow_map.vert")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "Resources/Shaders/empty.frag")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    GET_UNIFORM_AND_CHECK(m_WVPLoc, "gWVP");

    return true;
}



void ShadowMappingTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}