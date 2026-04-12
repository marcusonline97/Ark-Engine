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

	Ark::Editor::ImGuiLayer m_ImGui;
	EditorUI m_EditorUI;
	Logging::SinkId m_LogSinkId = 0;
	std::vector<EditorObject> m_Objects;
	int m_SelectedObject = -1;

	struct CachedTransformState
	{
		uint32_t objectId = 0;
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	};
	//Reused render input storage avoids per-frame fector reallocations.
	Ark::Rendering::WorldRenderInput m_renderInput{};

	// Per-object transform cache keyed by object index in m_Objects.
	std::vector<CachedTransformState> m_cachedObjectTransforms;
	std::vector<glm::mat4> m_cachedObjectModels;

	// Cached primary camera index, refreshed when scene topology/components change.
	size_t m_primaryCameraIndex = std::numeric_limits<size_t>::max();
	bool m_hasExplicitPrimaryCamera = false;
};

