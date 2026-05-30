#include "ButtonComponent.h"
#include "CanvasComponent.h"
#include "RectTransformComponent.h"
#include "Scene/GameObject.h"

namespace Engine
{
    void ButtonComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("color"))
        {
            auto& colorObj = json["color"];
            SetColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }

        if (json.contains("rect"))
        {
            auto& rectObj = json["rect"];
            m_rect = glm::vec2(
                rectObj.value("x", m_rect.x),
                rectObj.value("y", m_rect.y)
            );
        }

        if (json.contains("hovered"))
        {
            auto& colorObj = json["hovered"];
            SetHoveredColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }

        if (json.contains("pressed"))
        {
            auto& colorObj = json["pressed"];
            SetPressedColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            ));
        }
    }

    void ButtonComponent::SaveProperties(nlohmann::json& json) const
    {
        json["color"] = {
            {"r", m_color.r},
            {"g", m_color.g},
            {"b", m_color.b},
            {"a", m_color.a}
        };
        json["rect"] = {
            {"x", m_rect.x},
            {"y", m_rect.y}
        };
        json["hovered"] = {
            {"r", m_hoveredColor.r},
            {"g", m_hoveredColor.g},
            {"b", m_hoveredColor.b},
            {"a", m_hoveredColor.a}
        };
        json["pressed"] = {
            {"r", m_pressedColor.r},
            {"g", m_pressedColor.g},
            {"b", m_pressedColor.b},
            {"a", m_pressedColor.a}
        };
    }

    void ButtonComponent::Render(CanvasComponent* canvas)
    {
        if (!canvas)
        {
            return;
        }

		auto rt = GetOwner()->GetComponent<RectTransformComponent>();
		glm::vec2 ownerPos = GetOwner()->GetPosition2D();
        glm::vec2 size = m_rect;
        if (rt)
        {
            ownerPos = rt->GetScreenPosition();
            size = rt->GetSize();
		    ownerPos -= size * rt->GetPivot();
        }

        canvas->DrawRect(
            ownerPos,
            ownerPos + size,
            *m_currentColor
        );
    }

    bool ButtonComponent::HitTest(const glm::vec2& pos)
    {
		auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        glm::vec2 ownerPos = GetOwner()->GetPosition2D();
        glm::vec2 size = m_rect;
        if (rt)
        {
            ownerPos = rt->GetScreenPosition();
            size = rt->GetSize();
        }

		auto p1 = rt ? ownerPos - size * rt->GetPivot() : ownerPos;
		auto p2 = p1 + size;

        return (p1.x <= pos.x && p2.x >= pos.x && p1.y <= pos.y && p2.y >= pos.y);
    }

    void ButtonComponent::OnPointerEnter()
    {
        m_currentColor = &m_hoveredColor;
    }

    void ButtonComponent::OnPointerExit()
    {
        m_currentColor = &m_color;
    }

    void ButtonComponent::OnPointerUp()
    {
        m_currentColor = &m_hoveredColor;
    }

    void ButtonComponent::OnPointerDown()
    {
        m_currentColor = &m_pressedColor;
    }

    void ButtonComponent::OnClick()
    {
        if (onClick)
        {
            onClick();
        }
    }

    void ButtonComponent::SetColor(const glm::vec4& color)
    {
        m_color = color;
    }

    const glm::vec4& ButtonComponent::GetColor() const
    {
        return m_color;
    }

    void ButtonComponent::SetHoveredColor(const glm::vec4& color)
    {
        m_hoveredColor = color;
    }

    const glm::vec4& ButtonComponent::GetHoveredColor() const
    {
        return m_hoveredColor;
    }

    void ButtonComponent::SetPressedColor(const glm::vec4& color)
    {
        m_pressedColor = color;
    }

    const glm::vec4& ButtonComponent::GetPressedColor() const
    {
        return m_pressedColor;
    }
}