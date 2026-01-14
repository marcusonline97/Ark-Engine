#pragma once

#include <memory>
#include <vector>

#include "Logger.h"
#include "Editor/EditorUI.h"
#include "Editor/ImGuiLayer.h"
#include "Rendering/Threading/WorldRenderThread.h"

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
    std::vector<EditorObject> m_Objects;
    int m_SelectedObject = -1;
};
