#include "Material.h"
#include "Shader.h"
#include "Rendering/Texture/Texture.h"
#include <glad/glad.h>

void Material::Bind() const
{
    if (!m_Shader) return;
    m_Shader->Bind();

    if (m_UseTexture && m_Texture)
    {
        m_Texture->Bind(GL_TEXTURE0);
        m_Shader->SetInt("uTexture", 0);
        m_Shader->SetInt("uUseTexture", 1);
    }
    else
    {
        m_Shader->SetInt("uUseTexture", 0);
    }

    m_Shader->SetVec3("uTint", m_Tint);
}