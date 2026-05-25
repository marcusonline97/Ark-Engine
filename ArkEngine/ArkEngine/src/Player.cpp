#include "Player.h"
#include "Bullet.h"

#include <GLFW/glfw3.h> // For input bindings

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
            auto material = Engine::Material::Load("materials/suzanne.mat");
            auto mesh = Engine::Mesh::CreateSphere(0.2f, 32, 32);
            bullet->AddComponent(new Engine::MeshComponent(material, mesh));

            glm::vec3 pos = glm::vec3(0.0f);
            if (auto child = FindChildByName("BOOM_35"))
            {
                pos = child->GetWorldPosition();
            }
            bullet->SetPosition(pos + m_rotation * glm::vec3(-0.2f, 0.2f, -1.75f));

            auto collider = std::make_shared<Engine::SphereCollider>(0.2f);
            auto rigidBody = std::make_shared<Engine::RigidBody>(
                Engine::BodyType::Dynamic, collider, 10.0f, 0.1f);
            bullet->AddComponent(new Engine::PhysicsComponent(rigidBody));

            glm::vec3 front = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            rigidBody->ApplyImpulse(front * 500.0f);
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