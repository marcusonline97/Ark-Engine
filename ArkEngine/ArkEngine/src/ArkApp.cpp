#if defined(_WIN32)
#define WIN_32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <filesystem>


//ImGUI
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "Logger.h"
#include "AssetManager.h"
#include "Rendering/Framebuffer/Framebuffer.h"
#include "ImguiStyling.h"


static constexpr const char* kImGuiGLSLVersion = "#version 450";

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
        "ArkEngine/Resources/Shaders/vertex.glsl",
        "ArkEngine/Resources/Shaders/fragment.glsl"))
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
    m_Shader->SetVec3("u_Tint", glm::vec3(1.0f));
	m_Shader->SetInt("uTexture", 0); // Texture unit 0)

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	m_ImGuiInitialized = InitImGui();
    Logging::ToDo() << "Initializing ImGui.\n";

    m_ViewportShader = new Shader();
    if (!m_ViewportShader->LoadFromFiles(
        "ArkEngine/Resources/Shaders/vertex.glsl",
        "ArkEngine/Resources/Shaders/fragment.glsl"))
    {
        throw std::runtime_error("Failed to load viewport shader");
    }

    m_ViewportShader->Bind();
    m_ViewportShader->SetVec3("u_Tint", glm::vec3(1.0f));

    if (m_ImGuiInitialized)
    {
        m_EditorUI.Init();
        m_LogSinkId = Logging::AddSink([this](Logging::Level lvl, std::string_view msg)
            {
                m_EditorUI.PushLog(lvl, msg);
            });
    }

    // Minimal scene objects for the editor panels (until ECS/Scene is wired in)
    m_Objects.push_back(EditorObject{ "Cube" });
    m_Objects.push_back(EditorObject{ "Camera" });
    m_Objects.back().position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_Objects.push_back(EditorObject{ "Directional Light" });
    m_SelectedObject = 0;
    
}

App::~App()
{
    if (m_LogSinkId != 0)
    {
        Logging::RemoveSink(m_LogSinkId);
        m_LogSinkId = 0;
    }

    m_EditorUI.Shutdown();
	ShutDownImGui();

    delete m_ViewportShader; m_ViewportShader = nullptr;

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
        EditorObject* cubeObj = nullptr;
        if (!m_Objects.empty())
        {
            cubeObj = &m_Objects[0];
            if (cubeObj->name != "Cube")
            {
                for (auto& o : m_Objects)
                {
                    if (o.name == "Cube")
                    {
                        cubeObj = &o;
                        break;
                    }
                }
            }
        }

        glm::vec2 vpSize = m_EditorUI.GetViewportSize();
        int vpW = static_cast<int>(vpSize.x);
        int vpH = static_cast<int>(vpSize.y);
        if (vpW < 16 || vpH < 16)
        {
            // Fallback to window size until the viewport panel is laid out.
            glfwGetFramebufferSize(m_Window->GetNativeHandle(), &vpW, &vpH);
        }
        if (vpW < 1) vpW = 1;
        if (vpH < 1) vpH = 1;

        // (Re)allocate FBO if needed.
        if (m_ViewportFramebuffer.GetColorTextureId() == 0)
            m_ViewportFramebuffer.Create(static_cast<uint32_t>(vpW), static_cast<uint32_t>(vpH));
        else
            m_ViewportFramebuffer.Resize(static_cast<uint32_t>(vpW), static_cast<uint32_t>(vpH));

        // Render a simple scene (spinning cube) into the FBO.
        m_ViewportFramebuffer.Bind();
        glViewport(0, 0, vpW, vpH);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_ViewportShader && m_CubeMesh && m_Camera)
        {
            m_Camera->SetAspect(static_cast<float>(vpW) / static_cast<float>(vpH));

            if (!cubeObj || !cubeObj->enabled)
            {
                // nothing to render for the demo object
            }
            else
            {
                const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(cubeObj->rotationDeg.x), glm::vec3(1, 0, 0));
                const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(cubeObj->rotationDeg.y), glm::vec3(0, 1, 0));
                const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(cubeObj->rotationDeg.z), glm::vec3(0, 0, 1));
                const glm::mat4 rot = rotZ * rotY * rotX;
                const glm::mat4 model =
                    glm::translate(glm::mat4(1.0f), cubeObj->position) *
                    rot *
                    glm::scale(glm::mat4(1.0f), cubeObj->scale);

                const glm::mat4 mvp = m_Camera->GetViewProjection() * model;

                m_ViewportShader->Bind();
                m_ViewportShader->SetMat4("uMVP", mvp);
                m_ViewportShader->SetVec3("u_Tint", cubeObj->tint);
                m_CubeMesh->Draw();
            }

        }

        Framebuffer::Unbind();

        // Clear the main framebuffer before drawing UI.
        int winW = 0, winH = 0;
        glfwGetFramebufferSize(m_Window->GetNativeHandle(), &winW, &winH);
        glViewport(0, 0, winW, winH);
        glClearColor(0.12f, 0.12f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_ImGuiInitialized)
        {
            BeginImGuiFrame();
            m_EditorUI.SetViewportTextureId(m_ViewportFramebuffer.GetColorTextureId());
            m_EditorUI.Render(m_Objects, m_SelectedObject);
            EndImGuiFrame();
        }

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }
}


bool App::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Ark::EditorTheme::ApplyDarkTheme();

    if (!ImGui_ImplGlfw_InitForOpenGL(m_Window->GetNativeHandle(), true))
    {
        Logging::Error() << "ImGui_ImplGlfw_InitForOpenGL.\n";
        return false;
    }

    // Initialize OpenGL3 backend with GLSL version
    if (!ImGui_ImplOpenGL3_Init(kImGuiGLSLVersion))
    {
        Logging::Error() << "ImGui_ImplOpenGL3_Init failed.\n";
        return false;
    }
    return true;
}

void App::ShutDownImGui()
{
    if (!m_ImGuiInitialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_ImGuiInitialized = false;
}
void App::BeginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
void App::EndImGuiFrame()
{
    ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

