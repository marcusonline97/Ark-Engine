#pragma once

#include "Core/EngineInclude.h"

class GunPickup : public Engine::GameObject, public Engine::IContactListener
{
	GAMEOBJECT(GunPickup)

public:
	~GunPickup() override;

	void LoadProperties(const nlohmann::json& json) override;
	void Init() override;
	void Update(float deltaTime) override;
	void OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm) override;

private:
	Engine::RigidBody* m_rigidBody = nullptr;
	float m_rotationSpeedDegrees = 90.0f;
	bool m_pickedUp = false;
};