#include "Game.h"
#include "TestObject.h"

#include <iostream>

bool Game::Init()
{
	auto& fs = Engine::ArkEngine::GetInstance().GetFileSystem();
	auto texture = Engine::Texture::Load("brick.png");

    m_scene = new Engine::Scene();
    Engine::ArkEngine::GetInstance().SetScene(m_scene);

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new Engine::CameraComponent());
    camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    camera->AddComponent(new Engine::PlayerControllerComponent());

    m_scene->SetMainCamera(camera);

    m_scene->CreateObject<TestObject>("TestObject");

    auto material = Engine::Material::Load("materials/brick.mat");
    auto mesh = Engine::Mesh::CreateCube();

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

    auto gun = Engine::GameObject::LoadGLTF("models/CarbineGun/scene.gltf");
    gun->SetParent(camera);
    gun->SetPosition(glm::vec3(0.75f, -0.5f, -0.75f));
    gun->SetScale(glm::vec3(-1.0f, 1.0f, 1.0f));


    if (auto anim = gun->GetComponent<Engine::AnimationComponent>())
    {
        if (auto bullet = gun->FindChildByName("bullet_33"))
        {
            bullet->SetActive(false); // Spawn Bullet
        }

        if (auto fire = gun->FindChildByName("BOOM_35"))
        {
            fire->SetActive(false); // EFfect
        }

        anim->Play("shoot");
    }


    auto light = m_scene->CreateObject("Light");
    auto lightComp = new Engine::LightComponent();
    lightComp->SetColor(glm::vec3(1.0f));
    light->AddComponent(lightComp);
    light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));


    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}
