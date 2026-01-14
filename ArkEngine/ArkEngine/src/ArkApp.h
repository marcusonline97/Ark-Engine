#pragma once

#include <memory>

#include "Logger.h"
#include "Editor/EditorUI.h"
#include "Editor/ImGuiLayer.h"
#include "Rendering/Threading/WorldRenderThread.h"
#include "ECS/Scene.h"

struct GLFWwindow;

class ArkWindow;

class App
{
public:
    App();
    ~App();

    void Run();


    GLFWwindow* GetWindowHandle() const;
private:

    std::unique_ptr<ArkWindow> m_Window;
    std::unique_ptr<Ark::Rendering::WorldRenderThread> m_WorldRenderer;
    Ark::Editor::ImGuiLayer m_ImGui;

    EditorUI m_EditorUI;
    Logging::SinkId m_LogSinkId = 0;

    Ark::Scene m_Scene;
    entt::entity m_SelectedEntity = entt::null;
    entt::entity m_DemoCubeEntity = entt::null;
};
