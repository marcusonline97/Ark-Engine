#include <glad/glad.h>

#include "Material.h"
#include "Shader.h"
#include "Texture.h"

Material::Material() = default;
Material::~Material() = default;

void Material::SetShader(Shader* shader) { m_Shader = shader; }
Shader* Material::GetShader() const { return m_Shader; }

void Material::SetTexture(const Texture* texture)
{
    m_Texture = texture ? texture->GetId() : 0;
}

unsigned int Material::GetTexture() const { return m_Texture; }

void Material::SetUseTexture(bool use) { m_UseTexture = use; }
void Material::SetTint(const glm::vec3& tint) { m_Tint = tint; }

void Material::Bind() const
{
    if (!m_Shader) return;
    m_Shader->Bind();

    // Optional uniforms: only set if used by fragment shader
    m_Shader->SetFloat("uUseTexture", m_UseTexture ? 1.0f : 0.0f);
    m_Shader->SetVec3("uTint", m_Tint);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Texture);

    // Ensure sampler uniform exists; otherwise skip
    int samplerLoc = glGetUniformLocation(m_Shader->GetProgramId(), "uTexture");
    if (samplerLoc != -1) glUniform1i(samplerLoc, 0);
}