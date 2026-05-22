#pragma once
#include "Core/EngineInclude.h"

class Player : public Engine::GameObject
{
public:

	void Init();

	void Update(float deltaTime) override;

private:
	Engine::AnimationComponent* m_animationComponent = nullptr;
};