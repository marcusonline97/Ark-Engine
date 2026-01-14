#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"
#include "ArkWindow.h"


#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>



#include "Logger.h"
#include "Utility/Utility.h"
#include "ECS/Component.h"

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

    // Create a minimal ECS scene for the editor.
    {
        auto& reg = m_Scene.Registry();

        m_DemoCubeEntity = m_Scene.CreateEntity("Cube");
        reg.emplace<Ark::StaticMeshComponent>(m_DemoCubeEntity, Ark::StaticMeshComponent{});

        // Demo tint so materials panel has something to tweak.
        reg.get<Ark::StaticMeshComponent>(m_DemoCubeEntity).Tint = glm::vec3(1.0f, 1.0f, 1.0f);

        entt::entity cam = m_Scene.CreateEntity("Camera");
        reg.emplace<Ark::CameraComponent>(cam, Ark::CameraComponent{});
        reg.get<Ark::TransformComponent>(cam).Translation = glm::vec3(0.0f, 0.0f, 3.0f);
        m_Scene.SetPossessedCamera(cam);

        entt::entity light = m_Scene.CreateEntity("PointLight");
        reg.emplace<Ark::PointLightComponent>(light, Ark::PointLightComponent{});
        reg.get<Ark::TransformComponent>(light).Translation = glm::vec3(1.5f, 1.5f, 1.5f);
        reg.get<Ark::TransformComponent>(light).Scale = glm::vec3(0.2f);

        m_SelectedEntity = m_DemoCubeEntity;
    }
    
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
        auto& reg = m_Scene.Registry();

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

            // Camera: use possessed camera if present
            {
                const entt::entity camE = m_Scene.GetPossessedCamera();
                if (camE != entt::null && reg.valid(camE) && reg.all_of<Ark::TransformComponent, Ark::CameraComponent>(camE))
                {
                    const auto& t = reg.get<Ark::TransformComponent>(camE);
                    const auto& c = reg.get<Ark::CameraComponent>(camE);
                    input.camera.position = t.Translation;
                    input.camera.rotationDeg = t.Rotation;
                    input.camera.fov = c.FOV;
                    input.camera.nearPlane = c.NearPlane;
                    input.camera.farPlane = c.FarPlane;
                }
            }

            // Renderable objects: StaticMesh + SkeletalMesh (placeholder cube), plus PointLight icon cube
            input.objects.clear();
            input.objects.reserve(64);

            auto addObject = [&](entt::entity e, const glm::vec3& tintOverride)
            {
                if (!reg.valid(e) || !reg.any_of<Ark::TransformComponent>(e))
                    return;

                Ark::Rendering::WorldRenderObject o{};
                if (reg.any_of<Ark::EnabledComponent>(e))
                    o.enabled = reg.get<Ark::EnabledComponent>(e).Enabled;

                const auto& t = reg.get<Ark::TransformComponent>(e);
                o.position = t.Translation;
                o.rotationDeg = t.Rotation;
                o.scale = t.Scale;
                o.tint = tintOverride;
                input.objects.push_back(o);
            };

            reg.view<Ark::TransformComponent, Ark::StaticMeshComponent>().each([&](entt::entity e, const Ark::TransformComponent&, const Ark::StaticMeshComponent& sm)
            {
                addObject(e, sm.Tint);
            });

            reg.view<Ark::TransformComponent, Ark::SkeletalMeshComponent>().each([&](entt::entity e, const Ark::TransformComponent&, const Ark::SkeletalMeshComponent& sk)
            {
                addObject(e, sk.Tint);
            });

            reg.view<Ark::TransformComponent, Ark::PointLightComponent>().each([&](entt::entity e, const Ark::TransformComponent&, const Ark::PointLightComponent& pl)
            {
                // Light icon: keep small regardless of user scale (still respects it).
                if (!reg.valid(e) || !reg.any_of<Ark::TransformComponent>(e))
                    return;

                Ark::Rendering::WorldRenderObject o{};
                if (reg.any_of<Ark::EnabledComponent>(e))
                    o.enabled = reg.get<Ark::EnabledComponent>(e).Enabled;

                const auto& t = reg.get<Ark::TransformComponent>(e);
                o.position = t.Translation;
                o.rotationDeg = t.Rotation;
                o.scale = t.Scale * 0.25f;
                o.tint = pl.Color;
                input.objects.push_back(o);
            });

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
            m_EditorUI.SetViewportTextureId(m_WorldRenderer ? m_WorldRenderer->GetLatestTextureId() : 0);
            m_EditorUI.Render(m_Scene, m_SelectedEntity);
            m_ImGui.EndFrame();
        }

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }
}

