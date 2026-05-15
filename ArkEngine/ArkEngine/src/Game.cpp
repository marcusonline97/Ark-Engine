#include "Game.h"
#include "TestObject.h"

#include <iostream>


#define STB_IMAGE_IMPLEMENTATION
#include <Stb_image/stb_image.h>

bool Game::Init()
{
	auto& fs = Engine::ArkEngine::GetInstance().GetFileSystem();
	auto path = fs.GetAssetsFolder() / "Brick.png";

	int width, height, channels;
	unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

	if (data)
	{
		std::cout << "Image Loaded" << std::endl;
 	}

    m_scene = new Engine::Scene();

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new Engine::CameraComponent());
    camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	camera->AddComponent(new Engine::PlayerControllerComponent());

    m_scene->SetMainCamera(camera);

    m_scene->CreateObject<TestObject>("TestObject");

	std::string vertexShaderSource = R"(
		#version 330 core
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 color;

		out vec3 vColor;

		uniform mat4 uModel;
		uniform mat4 uView;
		uniform mat4 uProjection;


		void main()
		{
			vColor = color;
			gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
		}
	)";

	std::string fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		in vec3 vColor;
		void main()
		{
			FragColor = vec4(vColor, 1.0);
		}
	)";

	auto& graphicAPI = Engine::ArkEngine::GetInstance().GetGraphicsAPI();
	auto shaderProgram = graphicAPI.CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	auto material = std::make_shared<Engine::Material>();
	material->SetShaderProgram(shaderProgram);

	std::vector<float> vertices =
	{
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f,

		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f
	};

	std::vector<unsigned int> indices =
	{
		//front face
		0, 1, 2,
		0, 2, 3,
		//top face
		4, 5 ,1,
		4, 1, 0,
		//right face
		4, 0, 3,
		4, 3, 7,
		//left face
		1, 5 , 6,
		1, 6, 2,
		//botton face
		3, 2, 6,
		3, 6, 7,
		//back face
		4, 7 ,6,
		4, 6, 5
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

	vertexLayout.stride = sizeof(float) * 6;

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
