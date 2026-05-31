#include "LightComponent.h"

namespace Engine
{

    void LightComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("color"))
        {
            const auto& colorObj = json["color"];
            glm::vec3 color(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f)
            );
            SetColor(color);
        }
    }

    void LightComponent::SaveProperties(nlohmann::json& json) const
    {
        json["color"] = {
            {"r", m_color.r},
            {"g", m_color.g},
            {"b", m_color.b}
        };
    }

    void LightComponent::Update(float deltaTime)
    {

    }

    void LightComponent::SetColor(const glm::vec3& color)
    {
        m_color = color;
    }

    const glm::vec3& LightComponent::GetColor() const
    {
        return m_color;
    }
}