#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <Logger.h>
#include <Editor/EditorUI.h>
#include <Editor/ImGuiLayer.h>
#include <Rendering/Threading/WorldRenderThread.h>
#include <AssetManager/ResourceLoader.h>
#include <ArkPhysics.h>

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

	Ark::Rendering::ResourceLoader m_cpuResourceLoader;
	std::unique_ptr<Ark::Rendering::WorldRenderThread> m_WorldRenderer;
	std::unique_ptr<Ark::Physics::PhysicsThreadedWorld> m_PhysicsWorld;

	Ark::Editor::ImGuiLayer m_ImGui;
	EditorUI m_EditorUI;
	Logging::SinkId m_LogSinkId = 0;
	std::vector<EditorObject> m_Objects;
	int m_SelectedObject = -1;

};

