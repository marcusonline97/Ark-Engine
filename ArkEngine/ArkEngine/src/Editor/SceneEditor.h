#pragma once

#include "Scene/Scene.h"
#include "Logger.h"
#include "MemoryStats.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Engine
{
	enum class MipmapFilter
	{
		Linear,
		Nearest
	};

	class SceneEditor
	{
	public:
		SceneEditor();

		void SetScene(const std::shared_ptr<Scene>& scene, const std::string& scenePath);

		void SetActive(bool active);
		bool IsActive() const;

		void Render();
		void ClearSelection();

		MipmapFilter GetMipmapFilter() const { return m_mipmapFilter; }

		// Public so the static log-sink bridge in SceneEditor.cpp can push entries
		struct LogEntry
		{
			Logging::Level level = Logging::Level::DEBUG;
			std::string    message;
			float          timestamp = 0.0f;
		};
		std::vector<LogEntry> m_logEntries;

	private:
		// ── Layout ──────────────────────────────────────────────
		void DrawEditorDockspace();

		// ── Panels ──────────────────────────────────────────────
		void DrawViewport();
		void DrawHierarchy();
		void DrawObjectNode(GameObject* object);
		void DrawInspector();
		void DrawComponents(GameObject* object);
		void DrawBottomPanel();
		void DrawContentBrowser();
		void DrawRendering();
		void DrawLogs();
		void DrainPendingLogs();

		// ── Content browser helpers ──────────────────────────────
		void RefreshDirectory(const std::filesystem::path& dir);
		static const char* FileIcon(const std::filesystem::path& p);
		static bool IsImageFile(const std::filesystem::path& p);
		static bool IsMaterialFile(const std::filesystem::path& p);
		static bool IsSceneFile(const std::filesystem::path& p);
		static bool IsAudioFile(const std::filesystem::path& p);
		static bool IsMeshFile(const std::filesystem::path& p);

		// ── Rendering helpers ────────────────────────────────────
		std::size_t ApplyMipmapFilter(MipmapFilter filter);

		// ── Helpers ──────────────────────────────────────────────
		void SelectObject(GameObject* object);
		GameObject* CreateObject(GameObject* parent);
		void DeleteSelected();
		void SaveScene();

	private:
		// Scene
		std::shared_ptr<Scene> m_scene;
		std::string            m_scenePath = "Scenes/scene.sc";
		bool                   m_active = false;

		// Selection / inspector state
		GameObject* m_selectedObject = nullptr;
		GameObject* m_nameBufferObject = nullptr;
		int                    m_componentTypeIndex = 0;
		std::array<char, 128>  m_nameBuffer = {};
		std::array<char, 128>  m_newObjectName = { 'N','e','w',' ','O','b','j','e','c','t','\0' };

		// Status bar
		std::string            m_status;

		// Content Browser
		std::filesystem::path              m_contentRoot;
		std::filesystem::path              m_contentCurrent;
		std::vector<std::filesystem::path> m_contentEntries;
		std::string                        m_contentFilter;
		std::array<char, 128>              m_contentFilterBuf = {};

		// Rendering settings
		MipmapFilter           m_mipmapFilter = MipmapFilter::Linear;
		float                  m_memStatsPollTimer = 0.0f;
		Engine::MemoryStats    m_lastMemStats;

		// Logs panel
		bool                   m_logAutoScroll = true;
		bool                   m_logShowInit = true;
		bool                   m_logShowDebug = true;
		bool                   m_logShowWarning = true;
		bool                   m_logShowError = true;
		bool                   m_logShowFatal = true;
		bool                   m_logShowTodo = true;
		bool                   m_logShowFunction = true;

		// Dockspace – built once per activation
		bool m_dockspaceBuilt = false;
	};
}