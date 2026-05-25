#pragma once

#include "Core/EngineInclude.h"

class Bullet : public Engine::GameObject
{
	GAMEOBJECT(Bullet)
public:
	void Update(float deltaTime) override;

private:
	float m_lifetime = 2.0f; // Bullet will be destroyed after this time
};