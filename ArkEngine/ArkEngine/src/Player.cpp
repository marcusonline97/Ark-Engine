#include "Player.h"

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
        m_animationComponent = GetComponent<Engine::AnimationComponent>();

    }
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