#pragma once

#include "Scene/Component.h"

namespace Engine
{
	class PlayerControllerComponent : public Component
	{
		COMPONENT(PlayerControllerComponent)

	public:
		void Update(float deltaTime) override;
	
	private:

		float m_sensitivity = 1.5f;
		float m_moveSpeed = 1.0f;
	};
}