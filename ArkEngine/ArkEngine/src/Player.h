#pragma once
#include "Core/EngineInclude.h"

class Player : public Engine::GameObject
{
	GAMEOBJECT(Player)
public:

	void Init() override;

	void Update(float deltaTime) override;
	void PreloadBulletMaterial();
	const std::shared_ptr<Engine::Material>& GetBulletMaterial();
	void SetHasGun(bool hasGun);
	bool HasGun() const;

private:
	Engine::AnimationComponent* m_animationComponent = nullptr;
	Engine::AudioComponent* m_audioComponent = nullptr;
	Engine::PlayerControllerComponent* m_playerControllerComponent = nullptr;
	Engine::GameObject* m_gunObject = nullptr;
	std::shared_ptr<Engine::Material> m_bulletMaterial;
	bool m_hasGun = false;

};