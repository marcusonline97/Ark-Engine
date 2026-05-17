#include "Game.h"
#include "TestObject.h"

#include <iostream>



bool Game::Init()
{
	auto& fs = Engine::ArkEngine::GetInstance().GetFileSystem();
	auto texture = Engine::Texture::Load("brick.png");


    m_scene = new Engine::Scene();

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new Engine::CameraComponent());
    camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	camera->AddComponent(new Engine::PlayerControllerComponent());

    m_scene->SetMainCamera(camera);

    m_scene->CreateObject<TestObject>("TestObject");
	std::string vertexShaderSource = fs.LoadAssetFileText("Shaders/Vertex.glsl");
	std::string fragmentShaderSource = fs.LoadAssetFileText("Shaders/Fragment.glsl");

	auto& graphicAPI = Engine::ArkEngine::GetInstance().GetGraphicsAPI();
	auto shaderProgram = graphicAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	

	auto material = Engine::Material::Load("Materials/brick.mat");
	
	auto mesh = Engine::Mesh::CreateCube();

	auto objectA = m_scene->CreateObject("ObjectA");
	objectA->AddComponent(new Engine::MeshComponent(material, mesh));
	objectA->SetPosition(glm::vec3(1.0f, 0.0f, -5.0f));

	auto objectB = m_scene->CreateObject("ObjectB");
	objectB->AddComponent(new Engine::MeshComponent(material, mesh));
	objectB->SetPosition(glm::vec3(0.0f, 2.0f, 2.0f));
	objectB->SetRotation(glm::vec3(0.0f, 2.0f, 0.0f));

	auto objectC = m_scene->CreateObject("ObjectC");
	objectC->AddComponent(new Engine::MeshComponent(material, mesh));
	objectC->SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
	objectC->SetRotation(glm::vec3(1.0f, 0.0f, 1.0f));
	objectC->SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

	auto suzanneMesh = Engine::Mesh::Load("models/Suzanne.gltf");
	auto suzanneMaterial = Engine::Material::Load("materials/suzanne.mat");

	if (suzanneMesh && suzanneMaterial)
	{
		auto suzanneObj = m_scene->CreateObject("Suzanne");
		suzanneObj->AddComponent(new Engine::MeshComponent(suzanneMaterial, suzanneMesh));
		suzanneObj->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
	}
	else
	{
		printf("[Game::Init] WARNING: Suzanne mesh or material failed to load\n");
	}

	auto light = m_scene->CreateObject("Light");
	auto lightComp = new Engine::LightComponent();
	lightComp->SetColor(glm::vec3(1.0f));
	light->AddComponent(lightComp);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));

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
