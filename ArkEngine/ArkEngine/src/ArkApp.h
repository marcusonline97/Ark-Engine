#pragma once
#include "ArkWindow.h"
#include "Shader.h"
#include "Camera/Camera.h"
#include "Meshes/Cube.h"
#include "Material.h"
#include "Texture.h"

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
	bool m_ShowImGuiDemo = true;
};
