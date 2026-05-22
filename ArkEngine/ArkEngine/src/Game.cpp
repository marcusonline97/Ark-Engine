#include "Game.h"
#include "TestObject.h"
#include "Player.h"

#include <iostream>

bool Game::Init()
{
    auto& fs = Engine::ArkEngine::GetInstance().GetFileSystem();
    auto texture = Engine::Texture::Load("brick.png");

    m_scene = new Engine::Scene();
    Engine::ArkEngine::GetInstance().SetScene(m_scene);

    auto player = m_scene->CreateObject<Player>("Player");
    player->Init();
    m_scene->SetMainCamera(player);

    m_scene->CreateObject<TestObject>("TestObject");

    auto material = Engine::Material::Load("materials/brick.mat");
    auto mesh = Engine::Mesh::CreateBox();

    auto objectB = m_scene->CreateObject("ObjectB");
    objectB->AddComponent(new Engine::MeshComponent(material, mesh));
    objectB->SetPosition(glm::vec3(0.0f, 2.0f, 2.0f));
    objectB->SetRotation(glm::vec3(0.0f, 2.0f, 0.0f));

    auto objectC = m_scene->CreateObject("ObjectC");
    objectC->AddComponent(new Engine::MeshComponent(material, mesh));
    objectC->SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
    objectC->SetRotation(glm::vec3(1.0f, 0.0f, 1.0f));
    objectC->SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

    auto suzanneObject = Engine::GameObject::LoadGLTF("models/suzanne/Suzanne.gltf");
    suzanneObject->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

    auto light = m_scene->CreateObject("Light");
    auto lightComp = new Engine::LightComponent();
    lightComp->SetColor(glm::vec3(1.0f));
    light->AddComponent(lightComp);
    light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));


    auto ground = m_scene->CreateObject("Ground");
    ground->SetPosition(glm::vec3(0.0f, -5.0f, 0.0f));

    glm::vec3 groundExtents(20.0f, 2.0f, 20.0f);
    auto groundMesh = Engine::Mesh::CreateBox(groundExtents);
    ground->AddComponent(new Engine::MeshComponent(material, groundMesh));

    auto groundCollider = std::make_shared<Engine::BoxCollider>(groundExtents);
    auto groundBody = std::make_shared<Engine::RigidBody>(
        Engine::BodyType::Static, groundCollider, 0.0f, 0.5f);
    ground->AddComponent(new Engine::PhysicsComponent(groundBody));

    auto boxObj = m_scene->CreateObject("FallingBox");
    boxObj->AddComponent(new Engine::MeshComponent(material, mesh));
    boxObj->SetPosition(glm::vec3(0.0f, 2.0f, 2.0f));
    boxObj->SetRotation(glm::quat(glm::vec3(1.0f, 2.0f, 0.0f)));
    auto boxCollider = std::make_shared<Engine::BoxCollider>(glm::vec3(1.0f));
    auto boxBody = std::make_shared<Engine::RigidBody>(
        Engine::BodyType::Dynamic, boxCollider, 5.0f, 0.5f);
    boxObj->AddComponent(new Engine::PhysicsComponent(boxBody));

    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}
