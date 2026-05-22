#include "Player.h"

#include <GLFW/glfw3.h> // For input bindings
void Player::Init()
{
    AddComponent(new Engine::CameraComponent());
    SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    AddComponent(new Engine::PlayerControllerComponent());

    auto gun = Engine::GameObject::LoadGLTF("models/CarbineGun/scene.gltf");
    gun->SetParent(this);
    gun->SetPosition(glm::vec3(0.75f, -0.5f, -0.75f));
    gun->SetScale(glm::vec3(-1.0f, 1.0f, 1.0f));


    if (auto anim = gun->GetComponent<Engine::AnimationComponent>())
    {
        if (auto bullet = gun->FindChildByName("bullet_33"))
        {
            bullet->SetActive(false);
        }

        if (auto fire = gun->FindChildByName("BOOM_35"))
        {
            fire->SetActive(false);
        }

        anim->Play("shoot", false);
    }
    m_animationComponent = gun->GetComponent<Engine::AnimationComponent>();
}

void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

    auto& input = Engine::ArkEngine::GetInstance().GetInputManager();
    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        if (m_animationComponent && !m_animationComponent->IsPlaying())
        {
            m_animationComponent->Play("shoot", false);
        }
    }
}