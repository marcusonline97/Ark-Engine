#include "Game.h"
#include "TestObject.h"
#include "Player.h"
#include "Bullet.h"
#include "JumpPlatform.h"

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
    Engine::ArkEngine::GetInstance().SetScene(scene.get());

    return true;
}

void Game::Update(float deltaTime)
{
    if (!m_scene)
    {
        return;
    }
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}
