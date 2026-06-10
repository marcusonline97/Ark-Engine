#pragma once

#include "UIElementComponent.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <memory>
#include <string>

namespace Engine
{
    class Texture;

    class UIImageComponent : public UIElementComponent
    {
        COMPONENT_2(UIImageComponent, UIElementComponent)

    public:
        void LoadProperties(const nlohmann::json& json) override;
        void SaveProperties(nlohmann::json& json) const override;
        void Render(CanvasComponent* canvas) override;

        void SetTexturePath(const std::string& texturePath);
        const std::string& GetTexturePath() const;

        void SetColor(const glm::vec4& color);
        const glm::vec4& GetColor() const;

        void SetFillParent(bool fillParent);
        bool ShouldFillParent() const;

    private:
        std::shared_ptr<Texture> m_texture;
        std::string m_texturePath;
        glm::vec4 m_color = glm::vec4(1.0f);
        glm::vec2 m_lowerLeftUV = glm::vec2(0.0f);
        glm::vec2 m_upperRightUV = glm::vec2(1.0f);
        bool m_fillParent = false;
    };
}
