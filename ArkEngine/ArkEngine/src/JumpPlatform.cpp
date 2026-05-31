#include "JumpPlatform.h"

void JumpPlatform::Init()
{
    auto physics = GetComponent<Engine::PhysicsComponent>();
    if (physics)
    {
        auto rigidBody = physics->GetRigidBody();
        if (rigidBody)
        {
            rigidBody->AddContactListener(this);
        }
    }
}

void JumpPlatform::OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm)
{
    if (obj->GetCollisionObjectType() == Engine::CollisionObjectType::KinematicCharacterController)
    {
        auto controller = static_cast<Engine::KinematicCharacterController*>(obj);
        if (controller)
        {
            controller->Jump(glm::vec3(0.0f, 7.5f, 0.0f));
        }
    }
}