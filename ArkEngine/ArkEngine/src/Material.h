#pragma once
#include <glm/vec3.hpp>

class Shader;
class Texture;

class Material
{
public:
    Material();
    ~Material();

    void SetShader(Shader* shader);
    Shader* GetShader() const;

    // Consistent texture API: accept Texture*, store GL id
    void SetTexture(const Texture* texture);
    unsigned int GetTexture() const;

    void SetUseTexture(bool use);
    void SetTint(const glm::vec3& tint);

    void Bind() const;

private:
    Shader* m_Shader = nullptr;
    unsigned int m_Texture = 0;
    bool m_UseTexture = true;
    glm::vec3 m_Tint = glm::vec3(1.0f);
};