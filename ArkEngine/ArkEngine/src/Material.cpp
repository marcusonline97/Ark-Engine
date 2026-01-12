#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>

void Material::Bind()const
{
	if (!m_Shader) return;
	m_Shader->Bind();

	//Material uniforms
	if (m_UseTexture && m_Texture)
	{
		m_Texture->BindUnit(0);
		m_Shader->SetInt("u_Texture", 0);
		m_Shader->SetInt("u_UseTexture", 1);
	}

	else
	{
		m_Shader->SetInt("u_UseTexture", 0);
	}
	m_Shader->SetVec3("u_Tint", m_Tint);
}