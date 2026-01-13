#pragma once

#include <memory>
#include <vector>

#include "Logger.h"
#include "Editor/EditorUI.h"
#include "Editor/ImGuiLayer.h"
#include "Rendering/Framebuffer/Framebuffer.h"

struct GLFWwindow;

class ArkWindow;
class Shader;
class CubeMesh;
class ArkCamera;
class Material;
class Texture;

class App
{
public:
    App();
    ~App();

    void Run();


    GLFWwindow* GetWindowHandle() const;
private:

    std::unique_ptr<ArkWindow> m_Window;
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<CubeMesh> m_CubeMesh;
    std::unique_ptr<ArkCamera> m_Camera;
    std::unique_ptr<Material> m_Material;
    std::unique_ptr<Shader> m_ViewportShader;

    Texture* m_TextureObj = nullptr;


    Ark::Editor::ImGuiLayer m_ImGui;


    EditorUI m_EditorUI;
    Logging::SinkId m_LogSinkId = 0;
    std::vector<EditorObject> m_Objects;
    int m_SelectedObject = -1;

    Framebuffer m_ViewportFramebuffer;
};
