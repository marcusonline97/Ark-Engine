#pragma once
#include <glm/vec3.hpp>
class Texture;
class Shader;

class Material
{
public:
	Material() = default;
	~Material() = default;

	void SetShader(Shader* shader) { m_Shader = shader; }
	void SetTexture(Texture* texture) { m_Texture = texture; }
	void SetUseTexture(bool useTexture) { m_UseTexture = useTexture; }
	void SetTint(const glm::vec3& tint) { m_Tint = tint; }

	void Bind() const;

private:
	Shader* m_Shader = nullptr;
	Texture* m_Texture = nullptr;
	bool m_UseTexture = false;
	glm::vec3 m_Tint{ 1.0f, 1.0f, 1.0f };
};