#include "UIImageComponent.h"
#include "CanvasComponent.h"
#include "RectTransformComponent.h"
#include "Core/ArkEngine.h"
#include "Graphics/Texture.h"
#include "Scene/GameObject.h"

namespace Engine
{
    void UIImageComponent::LoadProperties(const nlohmann::json& json)
    {
        SetTexturePath(json.value("texture", ""));
        m_fillParent = json.value("fillParent", false);

        if (json.contains("color"))
        {
            const auto& colorObj = json["color"];
            m_color = glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            );
        }

        if (json.contains("lowerLeftUV"))
        {
            const auto& uvObj = json["lowerLeftUV"];
            m_lowerLeftUV = glm::vec2(
                uvObj.value("u", 0.0f),
                uvObj.value("v", 0.0f)
            );
        }

        if (json.contains("upperRightUV"))
        {
            const auto& uvObj = json["upperRightUV"];
            m_upperRightUV = glm::vec2(
                uvObj.value("u", 1.0f),
                uvObj.value("v", 1.0f)
            );
        }
    }

    void UIImageComponent::SaveProperties(nlohmann::json& json) const
    {
        json["texture"] = m_texturePath;
        json["fillParent"] = m_fillParent;
        json["color"] = {
            {"r", m_color.r},
            {"g", m_color.g},
            {"b", m_color.b},
            {"a", m_color.a}
        };
        json["lowerLeftUV"] = {
            {"u", m_lowerLeftUV.x},
            {"v", m_lowerLeftUV.y}
        };
        json["upperRightUV"] = {
            {"u", m_upperRightUV.x},
            {"v", m_upperRightUV.y}
        };
    }

    void UIImageComponent::Render(CanvasComponent* canvas)
    {
        if (!canvas || !m_texture)
        {
            return;
        }

        glm::vec2 lowerLeft(0.0f);
        glm::vec2 size(0.0f);

        if (m_fillParent)
        {
            if (auto parent = GetOwner()->GetParent())
            {
                if (auto parentRect = parent->GetComponent<RectTransformComponent>())
                {
                    size = parentRect->GetSize();
                    lowerLeft = parentRect->GetScreenPosition() - size * parentRect->GetPivot();
                }
            }
        }

        if (size.x <= 0.0f || size.y <= 0.0f)
        {
            if (auto rect = GetOwner()->GetComponent<RectTransformComponent>())
            {
                size = rect->GetSize();
                lowerLeft = rect->GetScreenPosition() - size * rect->GetPivot();
            }
            else
            {
                size = glm::vec2(
                    static_cast<float>(m_texture->GetWidth()),
                    static_cast<float>(m_texture->GetHeight())
                );
                lowerLeft = GetOwner()->GetPosition2D();
            }
        }

        if (size.x <= 0.0f || size.y <= 0.0f)
        {
            return;
        }

        canvas->DrawRect(
            lowerLeft,
            lowerLeft + size,
            m_lowerLeftUV,
            m_upperRightUV,
            m_texture.get(),
            m_color
        );
    }

    void UIImageComponent::SetTexturePath(const std::string& texturePath)
    {
        m_texturePath = texturePath;
        m_texture.reset();

        if (!m_texturePath.empty())
        {
            m_texture = ArkEngine::GetInstance().GetTextureManager().GetOrLoadTexture(m_texturePath);
        }
    }

    const std::string& UIImageComponent::GetTexturePath() const
    {
        return m_texturePath;
    }

    void UIImageComponent::SetColor(const glm::vec4& color)
    {
        m_color = color;
    }

    const glm::vec4& UIImageComponent::GetColor() const
    {
        return m_color;
    }

    void UIImageComponent::SetFillParent(bool fillParent)
    {
        m_fillParent = fillParent;
    }

    bool UIImageComponent::ShouldFillParent() const
    {
        return m_fillParent;
    }
}
