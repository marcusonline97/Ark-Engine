#include "UIElementComponent.h"

namespace Engine
{
	void UIElementComponent::Render(CanvasComponent* canvas)
	{
		// Base implementation does nothing. Derived classes will override this to render themselves.
	}

	bool UIElementComponent::HitTest(const glm::vec2& pos)
	{
		return false;
	}

	void UIElementComponent::OnPointerEnter()
	{

	}

	void UIElementComponent::OnPointerExit()
	{

	}

	void UIElementComponent::OnPointerUp()
	{

	}

	void UIElementComponent::OnPointerDown()
	{

	}

	void UIElementComponent::OnClick()
	{

	}
}