#include "Game.h"
#include "TestObject.h"



bool Game::Init()
{
    m_scene = new Engine::Scene();

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new Engine::CameraComponent());
    camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	camera->AddComponent(new Engine::PlayerControllerComponent());

    m_scene->SetMainCamera(camera);

    m_scene->CreateObject<TestObject>("TestObject");

    Engine::ArkEngine::GetInstance().SetScene(m_scene);

    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}
