#pragma once
#include "ArkWindow.h"
#include "Shader.h"
#include "Camera/Camera.h"
#include "Meshes/Cube.h"
#include "Material.h"
#include "Texture.h"
#include "Editor/EditorUI.h"
#include "Rendering/Framebuffer/Framebuffer.h"

#include <vector>

class App
{
public:
    App();
    ~App();

    void Run();


    GLFWwindow* GetWindowHandle() const { return m_Window ? m_Window->GetNativeHandle() : nullptr; }
private:

    bool InitImGui();
	void ShutDownImGui();
	void BeginImGuiFrame();
	void EndImGuiFrame();

    ArkWindow* m_Window = nullptr;
    Shader* m_Shader = nullptr;
    CubeMesh* m_CubeMesh = nullptr;
    ArkCamera* m_Camera = nullptr;

    Material* m_Material = nullptr;
    Texture* m_TextureObj = nullptr;

	bool m_ImGuiInitialized = false;

    EditorUI m_EditorUI;
    Logging::SinkId m_LogSinkId = 0;
    std::vector<EditorObject> m_Objects;
    int m_SelectedObject = -1;

    Framebuffer m_ViewportFramebuffer;
    Shader* m_ViewportShader = nullptr;
};
