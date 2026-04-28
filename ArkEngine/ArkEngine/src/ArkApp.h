#pragma once

#include <cstddef>
#include <limits>
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
	std::unique_ptr<ArkWindow>                          m_Window;

	// CPU-side resource loader for async texture/mesh streaming.
	Ark::Rendering::ResourceLoader                      m_cpuResourceLoader;

	// Render thread owns its own GL context and draws the scene into an FBO.
	std::unique_ptr<Ark::Rendering::WorldRenderThread>  m_WorldRenderer;

	// Physics runs on its own thread; transforms are consumed each frame.
	std::unique_ptr<Ark::Physics::PhysicsThreadedWorld> m_PhysicsWorld;

	// ImGui lifecycle wrapper and the editor UI built on top of it.
	Ark::Editor::ImGuiLayer m_ImGui;
	EditorUI                m_EditorUI;

	// Logging sink registered to forward engine logs into the editor console.
	Logging::SinkId m_LogSinkId = 0;

	// Scene state.
	std::vector<EditorObject> m_Objects;
	int                       m_SelectedObject = -1;
};