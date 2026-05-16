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
	

	std::vector<float> vertices =
	{
		// Front face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Top face 
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Right face
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Left face
			-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Bottom face
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Back face
			-0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f

	};

	std::vector<unsigned int> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,
		// top face
		4, 5, 6,
		4, 6, 7,
		// right face
		8, 9, 10,
		8, 10, 11,
		// left face
		12, 13, 14,
		12, 14, 15,
		// bottom face
		16, 17, 18,
		16, 18, 19,
		// back face
		20, 21, 22,
		20, 22, 23
	};

	Engine::VertexLayout vertexLayout;

	// position
	vertexLayout.elements.push_back({
		0,
		3,
		GL_FLOAT,
		0
		});
	//Color
	vertexLayout.elements.push_back({
		1,
		3,
		GL_FLOAT,
		sizeof(float) * 3
		});

	//UV
	vertexLayout.elements.push_back({
		2,
		2,
		GL_FLOAT,
		sizeof(float) * 6
		});
	vertexLayout.stride = sizeof(float) * 8;

	auto mesh = std::make_shared<Engine::Mesh>(vertexLayout, vertices, indices);


	auto objectA = m_scene->CreateObject("ObjectA");
	objectA->AddComponent(new Engine::MeshComponent(material, mesh));
	objectA->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));

	auto objectB = m_scene->CreateObject("ObjectB");
	objectB->AddComponent(new Engine::MeshComponent(material, mesh));
	objectB->SetPosition(glm::vec3(0.0f, 2.0f, 2.0f));
	objectB->SetRotation(glm::vec3(0.0f, 2.0f, 0.0f));

	auto objectC = m_scene->CreateObject("ObjectC");
	objectC->AddComponent(new Engine::MeshComponent(material, mesh));
	objectC->SetPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
	objectC->SetRotation(glm::vec3(1.0f, 2.0f, 0.0f));
	objectC->SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

	auto suzanneMesh = Engine::Mesh::Load("Models//Suzanne.gltf");
	auto suzanneMaterial = Engine::Material::Load("Materials/suzanne.mat");

	auto suzanneObj = m_scene->CreateObject("Suzanne");
	suzanneObj->AddComponent(new Engine::MeshComponent(suzanneMaterial, suzanneMesh));
	suzanneObj->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

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
