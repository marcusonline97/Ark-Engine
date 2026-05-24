#include "AudioListenerComponent.h"
#include "Scene/GameObject.h"
#include "Core/ArkEngine.h"

namespace Engine
{
	void AudioListenerComponent::Update(float deltaTime)
	{
		auto pos = m_owner->GetWorldPosition();
		ArkEngine::GetInstance().GetAudioManager().SetListenerPosition(pos);

	}

}