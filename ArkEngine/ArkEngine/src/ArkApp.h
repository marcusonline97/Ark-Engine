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
    /*
	bool InitImGui();
    void Begin();
	void End();
    void RenderImGui();
	*/ // Continue later if needed

    ArkWindow* m_Window = nullptr;
    Shader* m_Shader = nullptr;
    CubeMesh* m_CubeMesh = nullptr;
    ArkCamera* m_Camera = nullptr;

    Material* m_Material = nullptr;
    Texture* m_TextureObj = nullptr;
};
