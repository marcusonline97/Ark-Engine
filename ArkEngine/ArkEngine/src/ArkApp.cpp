#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <filesystem>

//Need Input
//IM GUI
//#include <imgui/imgui.h>
//#include <imgui/backends/imgui_impl_glfw.h>
//#include <imgui/backends/imgui_impl_opengl3.h>
#include "Logger.h"
#include "AssetManager.h"


App::App()
{
    m_Window = new ArkWindow(1400, 840, "Ark Engine");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(
        m_Window->GetNativeHandle(),
        [](GLFWwindow*, int width, int height)
        {
            glViewport(0, 0, width, height);
        }
    );

    m_Camera = new ArkCamera(
        glm::vec3(0.0f, 0.0f, 3.0f),
        45.0f,
        1280.0f / 720.0f,
        0.1f,
        100.0f
    );

    m_Shader = new Shader();
    if (!m_Shader->LoadFromFiles(
        "ArkEngine/src/Rendering/shaders/vertex.glsl",
        "ArkEngine/src/Rendering/shaders/fragment.glsl"))
    {
        throw std::runtime_error("Failed to load shader");
    }

    m_CubeMesh = new CubeMesh();

    m_Material = new Material();
    m_Material->SetShader(m_Shader);
    m_Material->SetUseTexture(true);
    m_Material->SetTint(glm::vec3(1.0f));

    // Use AssetManager for consistent path resolution and caching
    Texture* diffuse = AssetManager::Instance().LoadTexture2D("ArkEngine/Resources/Textures/BathroomFloor_ALB.png", true);

    if (!diffuse)
    {
		Logging::Error() << "Failed to load texture. Falling back to default Texture.\n";
		m_Material->SetUseTexture(false);

    }
    else
    {
		m_TextureObj = diffuse;
		m_Material->SetTexture(m_TextureObj);
    }

    m_Shader->Bind();
	m_Shader->SetInt("uTexture", 0); // Texture unit 0)

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
}

App::~App()
{
    delete m_CubeMesh;  m_CubeMesh = nullptr;
    delete m_Material;  m_Material = nullptr;

    delete m_Shader;    m_Shader = nullptr;
    delete m_Camera;    m_Camera = nullptr;
    delete m_Window;    m_Window = nullptr;

    // Do NOT delete AssetManager-owned textures
    m_TextureObj = nullptr;
}

void App::Run()
{
    while (!m_Window->ShouldClose())
    {
		int fbw = 0, fbh = 0;
		glfwGetFramebufferSize(m_Window->GetNativeHandle(), &fbw, &fbh);
        if(fbw > 0 && fbh > 0)
        {
			m_Camera->SetAspect(static_cast<float>(fbw) / static_cast<float>(fbh));

		}

		glViewport(0, 0, fbw, fbh);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float t = static_cast<float>(glfwGetTime());
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 mvp = m_Camera->GetViewProjection() * model;

        m_Material->Bind();
        m_Shader->SetMat4("uMVP", mvp);
        m_CubeMesh->Draw();

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }
}
/*
bool App::InitImGui()
{
    const char* glsl_Version = "#version 450";
    IMGUI_CHECKVERSION();

    if (!ImGui::CreateContext())
    {
        std::cerr << "Failed to create ImGui context" << std::endl;
        return false;
    }

}

void App::Begin()
{
}

void App::End()
{
}

void App::RenderImGui()
{
}

*/
