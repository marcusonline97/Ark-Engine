#include "ShootableCube.h"
#include "Bullet.h"

ShootableCube::~ShootableCube()
{
    if (m_rigidBody)
    {
        m_rigidBody->RemoveContactListener(this);
    }
}

void ShootableCube::Init()
{
    auto physics = GetComponent<Engine::PhysicsComponent>();
    if (!physics)
    {
        return;
    }

    const auto& rigidBody = physics->GetRigidBody();
    if (rigidBody)
    {
        m_rigidBody = rigidBody.get();
        m_rigidBody->AddContactListener(this);
    }
}

void ShootableCube::OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm)
{
    if (m_wasShot || !obj ||
        obj->GetCollisionObjectType() != Engine::CollisionObjectType::RigidBody)
    {
        return;
    }

    auto otherBody = static_cast<Engine::RigidBody*>(obj);
    auto bullet = dynamic_cast<Bullet*>(otherBody->GetOwner());
    if (!bullet)
    {
        return;
    }

    m_wasShot = true;
    bullet->MarkForDestroy();
    SetActive(false);
    MarkForDestroy();
}
