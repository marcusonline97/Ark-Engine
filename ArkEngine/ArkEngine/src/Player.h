#pragma once
#include "Core/EngineInclude.h"

class Player : public Engine::GameObject
{
	GAMEOBJECT(Player)
public:

	void Init() override;

	void Update(float deltaTime) override;

private:
	Engine::AnimationComponent* m_animationComponent = nullptr;
};