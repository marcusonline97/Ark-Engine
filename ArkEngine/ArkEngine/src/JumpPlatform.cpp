#include "JumpPlatform.h"

namespace
{
    constexpr float BounceCooldown = 0.2f;
	const glm::vec3 BounceVelocity(0.0f, 10.0f, 0.0f);
}

JumpPlatform::~JumpPlatform()
{
    if (m_rigidBody)
    {
        m_rigidBody->RemoveContactListener(this);
    }
}

void JumpPlatform::Init()
{
    auto physics = GetComponent<Engine::PhysicsComponent>();
    if (physics)
    {
        auto rigidBody = physics->GetRigidBody();
        if (rigidBody)
        {
            m_rigidBody = rigidBody.get();
            m_rigidBody->AddContactListener(this);
        }
    }
}

void JumpPlatform::Update(float deltaTime)
{
    if (m_bounceCooldownRemaining > 0.0f)
    {
        m_bounceCooldownRemaining -= deltaTime;
    }

    Engine::GameObject::Update(deltaTime);
}

void JumpPlatform::OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm)
{
        if (!obj || m_bounceCooldownRemaining > 0.0f || obj->GetCollisionObjectType() != Engine::CollisionObjectType::KinematicCharacterController)
        {
            return;
        }
    auto controller = static_cast<Engine::KinematicCharacterController*>(obj);
    controller->Launch(BounceVelocity);
}