#include "Game.h"
#include "TestObject.h"
#include "Player.h"
#include "Bullet.h"
#include "JumpPlatform.h"
#include "GunPickup.h"

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <cmath>
#include <iostream>

namespace
{
    class EditorCameraObject final : public Engine::GameObject
    {
    public:
        EditorCameraObject()
        {
            SetName("EditorCamera");
        }
    };
}
void Game::RegisterTypes()
{
	Player::Register();
	Bullet::Register();
	JumpPlatform::Register();
	GunPickup::Register();
}


bool Game::Init()
{
    
    auto scene = Engine::Scene::Load("Scenes/scene.sc");
    m_scene = scene;
	auto& engine = Engine::ArkEngine::GetInstance();
    engine.SetScene(m_scene);
    
    m_3DRoot = m_scene->FindObjectByName("3DRoot");
	m_mainPlayer = m_scene->FindObjectByName("MainPlayer");
    m_gameCamera = m_scene->GetMainCamera();

    if (m_3DRoot)
    {
        m_3DRoot->SetActive(false);
    }

	m_sceneEditor.SetScene(m_scene, "Scenes/scene.sc");

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
                    EnterPlayMode();
                };
        }
    }

    if (auto button = canvasComponent->GetOwner()->FindChildByName("EditButton"))
    {
        if (auto component = button->GetComponent<Engine::ButtonComponent>())
        {
            component->onClick = [this]()
                {
                    EnterEditMode();
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

    Engine::ArkEngine::GetInstance().GetGraphicsAPI().SetClearColor(117.0f / 256.0f, 187.0f / 256.0f, 253 / 256.0f, 1.0f);

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

    if (m_mode == Mode::Editing)
    {
        UpdateEditorCamera(deltaTime);
    }

    if (Engine::ArkEngine::GetInstance().GetInputManager().IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        if (m_mode == Mode::Playing || m_mode == Mode::Editing)
        {
            EnterMenuMode();
        }
    }
}

void Game::RenderUI()
{
    if (m_mode == Mode::Editing)
    {
        m_sceneEditor.Render();
    }
}

void Game::Destroy()
{

}


void Game::EnterMenuMode()
{
    auto& engine = Engine::ArkEngine::GetInstance();
    SetEditorCameraLookActive(false);
    RestoreGameCamera();
    if (auto canvas = engine.GetUIInputSystem().GetCanvas())
    {
        canvas->SetActive(true);
    }

    engine.GetUIInputSystem().SetActive(true);
    engine.SetCursorEnabled(true);
    m_sceneEditor.SetActive(false);

    if (m_mainPlayer)
    {
        m_mainPlayer->SetActive(true);
    }

    if (m_3DRoot)
    {
        m_3DRoot->SetActive(false);
    }

    m_mode = Mode::Menu;
}

void Game::EnterPlayMode()
{
    auto& engine = Engine::ArkEngine::GetInstance();
    SetEditorCameraLookActive(false);
    RestoreGameCamera();
    if (auto canvas = engine.GetUIInputSystem().GetCanvas())
    {
        canvas->SetActive(false);
    }

    engine.GetUIInputSystem().SetActive(false);
    engine.SetCursorEnabled(false);
    m_sceneEditor.SetActive(false);

    if (m_mainPlayer)
    {
        m_mainPlayer->SetActive(true);
    }

    if (m_3DRoot)
    {
        m_3DRoot->SetActive(true);
    }

    m_mode = Mode::Playing;
}

void Game::EnterEditMode()
{
    auto& engine = Engine::ArkEngine::GetInstance();
    EnsureEditorCamera();
    if (m_scene)
    {
        if (auto currentCamera = m_scene->GetMainCamera())
        {
            if (currentCamera != m_editorCamera.get())
            {
                m_gameCamera = currentCamera;
            }
        }
    }
    SyncEditorCameraFrom(m_gameCamera);

    if (auto canvas = engine.GetUIInputSystem().GetCanvas())
    {
		canvas->SetActive(false);
    }
	engine.GetUIInputSystem().SetActive(false);
    engine.SetCursorEnabled(true);

    if (m_3DRoot)
    {
		m_3DRoot->SetActive(true);
    }

    if (m_mainPlayer)
    {
		m_mainPlayer->SetActive(false);
    }

    m_sceneEditor.SetActive(true);
    m_mode = Mode::Editing;
}

void Game::EnsureEditorCamera()
{
    if (m_editorCamera)
    {
        return;
    }

    m_editorCamera = std::make_unique<EditorCameraObject>();
    m_editorCamera->AddComponent(new Engine::CameraComponent());
    m_editorCamera->SetPosition(glm::vec3(0.0f, 2.0f, 5.0f));
}

void Game::RestoreGameCamera()
{
    if (m_scene && m_gameCamera)
    {
        m_scene->SetMainCamera(m_gameCamera);
    }
}

void Game::SetEditorCameraLookActive(bool active)
{
    if (m_editorCameraLookActive == active)
    {
        return;
    }

    m_editorCameraLookActive = active;
    m_skipEditorCameraMouseDelta = active;
    Engine::ArkEngine::GetInstance().SetCursorEnabled(!active);
}

void Game::SyncEditorCameraFrom(Engine::GameObject* camera)
{
    if (!m_editorCamera || !camera)
    {
        return;
    }

    m_editorCamera->SetPosition(camera->GetWorldPosition());
    m_editorCamera->SetRotation(camera->GetWorldRotation());

    glm::vec3 forward = m_editorCamera->GetRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
    forward = glm::normalize(forward);
    m_editorCameraPitch = glm::degrees(std::asin(glm::clamp(forward.y, -1.0f, 1.0f)));
    m_editorCameraYaw = glm::degrees(std::atan2(-forward.x, -forward.z));
}

void Game::UpdateEditorCamera(float deltaTime)
{
    if (!m_editorCamera)
    {
        return;
    }

    auto& inputManager = Engine::ArkEngine::GetInstance().GetInputManager();
    const bool lookActive = inputManager.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
    SetEditorCameraLookActive(lookActive);

    if (!m_editorCameraLookActive)
    {
        return;
    }

    constexpr float mouseSensitivity = 10.0f;
    constexpr float moveSpeed = 10.0f;
    constexpr float fastMoveMultiplier = 4.0f;

    if (inputManager.IsMousePositionChanged())
    {
        if (m_skipEditorCameraMouseDelta)
        {
            m_skipEditorCameraMouseDelta = false;
        }
        else
        {
            const auto& oldPos = inputManager.GetMousePositionOld();
            const auto& currentPos = inputManager.GetMousePositionCurrent();

            m_editorCameraYaw -= (currentPos.x - oldPos.x) * mouseSensitivity * deltaTime;
            m_editorCameraPitch -= (currentPos.y - oldPos.y) * mouseSensitivity * deltaTime;
            m_editorCameraPitch = glm::clamp(m_editorCameraPitch, -89.0f, 89.0f);
        }
    }

    const glm::quat yawRot = glm::angleAxis(glm::radians(m_editorCameraYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat pitchRot = glm::angleAxis(glm::radians(m_editorCameraPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::quat rotation = glm::normalize(yawRot * pitchRot);
    m_editorCamera->SetRotation(rotation);

    const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 move(0.0f);
    if (inputManager.IsKeyPressed(GLFW_KEY_W))
    {
        move += forward;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_S))
    {
        move -= forward;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_D))
    {
        move += right;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_A))
    {
        move -= right;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_E) || inputManager.IsKeyPressed(GLFW_KEY_SPACE))
    {
        move += worldUp;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_Q) || inputManager.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
    {
        move -= worldUp;
    }

    if (glm::dot(move, move) > 0.0f)
    {
        float speed = moveSpeed;
        if (inputManager.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
        {
            speed *= fastMoveMultiplier;
        }

        const glm::vec3 position = m_editorCamera->GetPosition() + glm::normalize(move) * speed * deltaTime;
        m_editorCamera->SetPosition(position);
    }
}