#include "Player.h"
#include "Bullet.h"

#include <GLFW/glfw3.h> // For input bindings

namespace
{
    constexpr float BulletRadius = 0.2f;
    constexpr int BulletSectors = 32;
    constexpr int BulletStacks = 32;
    constexpr float BulletMass = 10.0f;
    constexpr float BulletFriction = 0.1f;
    constexpr float BulletImpulse = 500.0f;
    constexpr const char* BulletMaterialPath = "materials/suzanne.mat";
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

    PreloadBulletAssets();
}

void Player::PreloadBulletAssets()
{
    m_bulletMaterial = Engine::Material::Load(BulletMaterialPath);
    m_bulletMesh = Engine::Mesh::CreateSphere(BulletRadius, BulletSectors, BulletStacks);
    m_bulletCollider = std::make_shared<Engine::SphereCollider>(BulletRadius);
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

            auto bullet = m_scene->CreateObject<Bullet>("Bullet");
            bullet->AddComponent(new Engine::MeshComponent(m_bulletMaterial, m_bulletMesh));

            glm::vec3 pos = glm::vec3(0.0f);
            if (auto child = FindChildByName("BOOM_35"))
            {
                pos = child->GetWorldPosition();
            }
            bullet->SetPosition(pos + m_rotation * glm::vec3(-0.2f, 0.2f, -1.75f));

            auto rigidBody = std::make_shared<Engine::RigidBody>(
                Engine::BodyType::Dynamic, m_bulletCollider, BulletMass, BulletFriction);
            bullet->AddComponent(new Engine::PhysicsComponent(rigidBody));

            glm::vec3 front = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            rigidBody->ApplyImpulse(front * BulletImpulse);
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