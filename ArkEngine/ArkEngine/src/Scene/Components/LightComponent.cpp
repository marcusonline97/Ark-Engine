#include "LightComponent.h"

#include <string>

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
        const std::string type = json.value("type", "directional");
        SetLightType(type == "point" ? LightType::Point : LightType::Directional);
        SetIntensity(json.value("intensity", 1.0f));
		SetRange(json.value("range", 10.0f));
    }

    void LightComponent::SaveProperties(nlohmann::json& json) const
    {
        json["color"] = {
            {"r", m_color.r},
            {"g", m_color.g},
            {"b", m_color.b}
        };
        json["type"] = m_type == LightType::Point ? "point" : "directional";
        json["intensity"] = m_intensity;
        json["range"] = m_range;
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

    void LightComponent::SetLightType(LightType type)
    {
        m_type = type;
    }

    LightType LightComponent::GetLightType() const
    {
        return m_type;
    }

    void LightComponent::SetIntensity(float intensity)
    {
        m_intensity = intensity;
    }

    float LightComponent::GetIntensity() const
    {
        return m_intensity;
    }

    void LightComponent::SetRange(float range)
    {
        m_range = range;
    }

    float LightComponent::GetRange() const
    {
        return m_range;
    }
}