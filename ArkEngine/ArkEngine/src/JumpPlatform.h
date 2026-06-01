#pragma once

#include <Core/EngineInclude.h>

class JumpPlatform : public Engine::GameObject, public Engine::IContactListener
{
    GAMEOBJECT(JumpPlatform)
public:
	~JumpPlatform() override;

    void Init() override;
	void Update(float deltaTime) override;
    void OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm) override;

private:
	Engine::RigidBody* m_rigidBody = nullptr;
	float m_bounceCooldownRemaining = 0.0f;
};