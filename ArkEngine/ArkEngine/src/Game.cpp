#include "Game.h"
#include "TestObject.h"
#include "Player.h"

#include <iostream>

void Game::RegisterTypes()
{
	Player::Register();
}


bool Game::Init()
{
    auto scene = Engine::Scene::Load("Scenes/scene.sc");
    if (!scene)
    {
        std::cerr << "[Game] Failed to load scene. Check that 'Scenes/scene.sc' exists and is valid JSON.\n";
        return false;
    }

    m_scene = scene;
    Engine::ArkEngine::GetInstance().SetScene(scene.get());

    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}
