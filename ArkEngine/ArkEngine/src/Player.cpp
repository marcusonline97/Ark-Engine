#include "Player.h"
#include "Bullet.h"

#include <GLFW/glfw3.h> // For input bindings

namespace
{
    constexpr float BulletRadius = 0.2f;
    constexpr int BulletSphereSectors = 32;
    constexpr int BulletSphereStacks = 32;
    constexpr float BulletMass = 10.0f;
    constexpr float BulletFriction = 0.1f;
    constexpr float BulletImpulse = 500.0f;
    const glm::vec3 BulletMuzzleOffset(-0.2f, 0.2f, -1.75f);
}

void Player::Init()
{
    if (auto bullet = FindChildByName("bullet_33"))
    {
        bullet->SetActive(false);
    }

    if (auto fire = FindChildByName("BOOM_35"))
    {
        fire->SetActive(false);
    }

    if (auto gun = FindChildByName("Gun"))
    {
        m_animationComponent = gun->GetComponent<Engine::AnimationComponent>();
    }

    m_audioComponent = GetComponent<Engine::AudioComponent>();
    m_playerControllerComponent = GetComponent<Engine::PlayerControllerComponent>();
    EnsureBulletResources();
}

void Player::Update(float deltaTime)
{
    Engine::GameObject::Update(deltaTime);

    auto& input = Engine::ArkEngine::GetInstance().GetInputManager();
    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        if (m_animationComponent && !m_animationComponent->IsPlaying())
        {
            m_animationComponent->Play("shoot", false);

            if (m_audioComponent)
            {
                if (m_audioComponent->IsPlaying("shoot"))
                {
                    m_audioComponent->Stop("shoot");
                }
                m_audioComponent->Play("shoot");
            }

            FireBullet();
        }
    }

    if (input.IsKeyPressed(GLFW_KEY_SPACE))
    {
        if (m_audioComponent && !m_audioComponent->IsPlaying("jump"))
        {
            m_audioComponent->Play("jump");
        }
    }

    bool walking =
        input.IsKeyPressed(GLFW_KEY_W) ||
        input.IsKeyPressed(GLFW_KEY_A) ||
        input.IsKeyPressed(GLFW_KEY_S) ||
        input.IsKeyPressed(GLFW_KEY_D);

    if (walking && m_playerControllerComponent && m_playerControllerComponent->OnGround())
    {
        if (m_audioComponent && !m_audioComponent->IsPlaying("step"))
        {
            m_audioComponent->Play("step", true);
        }
    }
    else
    {
        if (m_audioComponent && m_audioComponent->IsPlaying("step"))
        {
            m_audioComponent->Stop("step");
        }
    }
}

void Player::EnsureBulletResources()
{
    if (m_bulletResourcesInitialized)
    {
        return;
    }

    m_bulletMaterial = Engine::Material::Load("materials/suzanne.mat");
    m_bulletMesh = Engine::Mesh::CreateSphere(BulletRadius, BulletSphereSectors, BulletSphereStacks);
    m_bulletCollider = std::make_shared<Engine::SphereCollider>(BulletRadius);
    m_bulletResourcesInitialized = true;
}

void Player::FireBullet()
{
    EnsureBulletResources();

    auto bullet = m_scene->CreateObject<Bullet>("Bullet");
    if (!bullet)
    {
        return;
    }

    if (m_bulletMaterial && m_bulletMesh)
    {
        bullet->AddComponent(new Engine::MeshComponent(m_bulletMaterial, m_bulletMesh));
    }

    glm::vec3 pos = glm::vec3(0.0f);
    if (auto child = FindChildByName("BOOM_35"))
    {
        pos = child->GetWorldPosition();
    }
    bullet->SetPosition(pos + m_rotation * BulletMuzzleOffset);

    auto rigidBody = std::make_shared<Engine::RigidBody>(
        Engine::BodyType::Dynamic, m_bulletCollider, BulletMass, BulletFriction);
    bullet->AddComponent(new Engine::PhysicsComponent(rigidBody));

    glm::vec3 front = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    rigidBody->ApplyImpulse(front * BulletImpulse);
}