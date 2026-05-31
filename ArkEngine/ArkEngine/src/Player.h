#pragma once
#include "Core/EngineInclude.h"

class Player : public Engine::GameObject
{
	GAMEOBJECT(Player)
public:

	void Init() override;

	void Update(float deltaTime) override;

private:
	void EnsureBulletResources();
	void FireBullet();

	Engine::AnimationComponent* m_animationComponent = nullptr;
	Engine::AudioComponent* m_audioComponent = nullptr;
	Engine::PlayerControllerComponent* m_playerControllerComponent = nullptr;
	std::shared_ptr<Engine::Material> m_bulletMaterial;
	std::shared_ptr<Engine::Mesh> m_bulletMesh;
	std::shared_ptr<Engine::Collider> m_bulletCollider;
	bool m_bulletResourcesInitialized = false;
};