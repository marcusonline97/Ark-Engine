#pragma once
#include "ArkWindow.h"
#include "Shader.h"
#include "Camera/Camera.h"
#include "Meshes/Cube.h"
#include "Material.h"
#include "Texture.h"
#include "Editor/EditorUI.h"

#include <vector>

class App
{
public:
    App();
    ~App();

    void Run();

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

    // Editor
    EditorUI m_EditorUI;
    Logging::SinkId m_LogSinkId = 0;
    std::vector<EditorObject> m_Objects;
    int m_SelectedObject = -1;
};
