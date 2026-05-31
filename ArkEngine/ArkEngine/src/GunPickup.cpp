#include "GunPickup.h"
#include "Player.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

GunPickup::~GunPickup()
{
    if (m_rigidBody)
    {
        m_rigidBody->RemoveContactListener(this);
    }
}

void GunPickup::LoadProperties(const nlohmann::json& json)
{
    m_rotationSpeedDegrees = json.value("rotationSpeed", m_rotationSpeedDegrees);
}

void GunPickup::Init()
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

void GunPickup::Update(float deltaTime)
{
    if (!IsActive())
    {
        return;
    }

    if (!m_pickedUp)
    {
        const float angle = glm::radians(m_rotationSpeedDegrees) * deltaTime;
        const glm::quat spin = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
        SetRotation(glm::normalize(spin * GetRotation()));
    }

    Engine::GameObject::Update(deltaTime);
}

void GunPickup::OnContact(Engine::CollisionObject* obj, const glm::vec3& pos, const glm::vec3& norm)
{
    if (m_pickedUp || !obj ||
        obj->GetCollisionObjectType() != Engine::CollisionObjectType::KinematicCharacterController)
    {
        return;
    }

    auto scene = GetScene();
    if (!scene)
    {
        return;
    }

    auto player = dynamic_cast<Player*>(scene->FindObjectByName("MainPlayer"));
    if (!player)
    {
        return;
    }

    player->SetHasGun(true);
    m_pickedUp = true;
    SetActive(false);
    MarkForDestroy();
}
