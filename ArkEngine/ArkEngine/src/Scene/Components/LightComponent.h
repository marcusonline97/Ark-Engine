#pragma once

#include "Scene/Component.h"

#include <glm/vec3.hpp>

namespace Engine
{
    enum class LightType
    {
        Directional = 0,
        Point = 1
    };

    class LightComponent : public Component
    {
        COMPONENT(LightComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;
        void SaveProperties(nlohmann::json& json) const override;
        void Update(float deltaTime) override;

        void SetColor(const glm::vec3& color);
        const glm::vec3& GetColor() const;

        void SetLightType(LightType type);
        LightType GetLightType() const;

        void SetIntensity(float intensity);
        float GetIntensity() const;

        void SetRange(float range);
        float GetRange() const;

    private:
        glm::vec3 m_color = glm::vec3(1.0f);
        LightType m_type = LightType::Directional;
        float m_intensity = 1.0f;
        float m_range = 10.0f;
    };
}