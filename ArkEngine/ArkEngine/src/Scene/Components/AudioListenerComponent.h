#pragma once

#include "Scene/Component.h"

namespace Engine
{
	class AudioListenerComponent : public Component
	{
		COMPONENT(AudioListenerComponent)
	public:
		//void LoadProperties(const nlohmann::json& json) override;
		void Update(float deltaTime) override;
	private:
	};
}