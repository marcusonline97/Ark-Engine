#include "Game.h"
#include "TestObject.h"
#include "Player.h"
#include "Bullet.h"
#include "JumpPlatform.h"

#include <GLFW/glfw3.h>
#include <iostream>

void Game::RegisterTypes()
{
	Player::Register();
	Bullet::Register();
	JumpPlatform::Register();
}


bool Game::Init()
{
    
    auto scene = Engine::Scene::Load("Scenes/scene.sc");
    m_scene = scene;
	auto& engine = Engine::ArkEngine::GetInstance();
    engine.SetScene(m_scene);
    
    m_3DRoot = m_scene->FindObjectByName("3DRoot");
    if (m_3DRoot)
    {
        m_3DRoot->SetActive(false);
    }

    auto canvasComponent = engine.GetUIInputSystem().GetCanvas();
    if (!canvasComponent)
    {
        return false;
    }
    canvasComponent->SetActive(true);
    engine.SetCursorEnabled(true);
    engine.GetUIInputSystem().SetActive(true);

    if (auto button = canvasComponent->GetOwner()->FindChildByName("PlayButton"))
    {
        if (auto component = button->GetComponent<Engine::ButtonComponent>())
        {
            component->onClick = [this]()
                {
                    auto& engine = Engine::ArkEngine::GetInstance();
                    engine.GetUIInputSystem().GetCanvas()->SetActive(false);
                    engine.SetCursorEnabled(false);

                    if (m_3DRoot)
                    {
                        m_3DRoot->SetActive(true);
                    }
                };
        }
    }

    if (auto button = canvasComponent->GetOwner()->FindChildByName("QuitButton"))
    {
        if (auto component = button->GetComponent<Engine::ButtonComponent>())
        {
            component->onClick = [this]()
                {
                    SetNeedsToBeClosed(true);
                };
        }
    }

    /* UI TEST
	m_scene = std::make_shared<Engine::Scene>();
	Engine::ArkEngine::GetInstance().SetScene(m_scene.get());

    auto sprite = m_scene->CreateObject("Sprite");
    auto spriteComponent = new Engine::SpriteComponent();
    auto texture = Engine::Texture::Load("Textures/BathroomFloor_ALB.png");
    spriteComponent->SetTexture(texture);

    sprite->AddComponent(spriteComponent);
    sprite->SetPosition2D(glm::vec2(500.0f, 500.0f));

    spriteComponent->SetSize(glm::vec2(200.0f, 100.0f));
    spriteComponent->SetUpperRightUV(glm::vec2(2.0f, 1.0f));
    sprite->SetRotation2D(glm::radians(45.0f));

    auto camera = m_scene->CreateObject("Camera");
    auto cameraComponent = new Engine::CameraComponent();
    camera->AddComponent(cameraComponent);
    m_scene->SetMainCamera(camera);

    auto canvas = m_scene->CreateObject("Canvas");
    auto canvasComponent = new Engine::CanvasComponent();
    canvas->AddComponent(canvasComponent);

	auto& uiInput = Engine::ArkEngine::GetInstance().GetUIInputSystem();
    uiInput.SetActive(true);
    uiInput.SetCanvas(canvasComponent);


	auto button = m_scene->CreateObject("Button", canvas);
    button->SetPosition2D(glm::vec2(300.0f, 300.0f));
	auto buttonComponent = new Engine::ButtonComponent();
    buttonComponent->SetRect(glm::vec2(150.0f, 50.0f));
	buttonComponent->SetColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    button->AddComponent(buttonComponent);


    auto text = m_scene->CreateObject("Text", canvas);
    text->SetPosition2D(glm::vec2(300.0f, 300.0f));
    auto textComponent = new Engine::TextComponent();
    text->AddComponent(textComponent);
    textComponent->SetText("Some Text");
    textComponent->SetFont("fonts/arial.ttf", 24);
    textComponent->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    */

    return true;
}

void Game::Update(float deltaTime)
{

    m_scene->Update(deltaTime);

	auto& engine = Engine::ArkEngine::GetInstance();

    if (Engine::ArkEngine::GetInstance().GetInputManager().IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        if (m_3DRoot && m_3DRoot->IsActive())
        {
			engine.GetUIInputSystem().GetCanvas()->SetActive(true);
            engine.SetCursorEnabled(true);
			m_3DRoot->SetActive(false);
        }
    }
}

void Game::Destroy()
{

}
