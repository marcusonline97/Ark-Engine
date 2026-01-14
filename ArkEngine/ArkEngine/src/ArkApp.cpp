#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"
#include "ArkWindow.h"


#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>



#include "Logger.h"
#include "Utility/Utility.h"

static constexpr const char* kImGuiGLSLVersion = "#version 450";

GLFWwindow* App::GetWindowHandle() const
{
    return m_Window ? m_Window->GetNativeHandle() : nullptr;
}

App::App()
{
    m_Window = std::make_unique<ArkWindow>(1400, 840, "Ark Engine");

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

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    const bool imguiOk = m_ImGui.Init(m_Window->GetNativeHandle(), kImGuiGLSLVersion);
    Logging::ToDo() << "Initializing ImGui.\n";

    if (imguiOk)
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
    
    m_WorldRenderer = std::make_unique<Ark::Rendering::WorldRenderThread>(m_Window->GetNativeHandle());
}

App::~App()
{
    if (m_LogSinkId != 0)
    {
        Logging::RemoveSink(m_LogSinkId);
        m_LogSinkId = 0;
    }

    m_EditorUI.Shutdown();
	m_ImGui.Shutdown();
    m_WorldRenderer.reset();
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

        if (m_WorldRenderer)
        {
            Ark::Rendering::WorldRenderInput input{};
            input.width = static_cast<uint32_t>(vpW);
            input.height = static_cast<uint32_t>(vpH);
            input.cubeEnabled = (cubeObj != nullptr) && cubeObj->enabled;
            if (cubeObj)
            {
                input.position = cubeObj->position;
                input.rotationDeg = cubeObj->rotationDeg;
                input.scale = cubeObj->scale;
                input.tint = cubeObj->tint;
            }
            m_WorldRenderer->Submit(input);
        }

        Utilities::TickViewportFPS(glfwGetTime());


        // Clear the main framebuffer before drawing UI.
        int winW = 0, winH = 0;
        glfwGetFramebufferSize(m_Window->GetNativeHandle(), &winW, &winH);
        glViewport(0, 0, winW, winH);
        glClearColor(0.12f, 0.12f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_ImGui.IsInitialized())
        {
            m_ImGui.BeginFrame();
            m_EditorUI.SetViewportTextureId(m_WorldRenderer ? m_WorldRenderer->GetLatestTextureId() : 0);            m_EditorUI.Render(m_Objects, m_SelectedObject);
            m_ImGui.EndFrame();
        }

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }
}

