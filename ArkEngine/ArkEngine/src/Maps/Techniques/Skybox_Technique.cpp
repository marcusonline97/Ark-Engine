#include <limits.h>
#include <string.h>

#include "SkyBox_Technique.h"
#include "Utility/util.h"


SkyboxTechnique::SkyboxTechnique()
{
}

bool SkyboxTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/skybox.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/skybox.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_textureLoc = GetUniformLocation("gCubemapTexture");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_textureLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}


void SkyboxTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void SkyboxTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_textureLoc, TextureUnit);
}