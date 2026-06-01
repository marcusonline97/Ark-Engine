#pragma once

#include "Scene/Scene.h"

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
	class SceneEditor
	{
	public:
		void SetScene(const std::shared_ptr<Scene>& scene, const std::string& scenePath);

		void SetActive(bool active);
		bool IsActive() const;

		void Render();
		void ClearSelection();

	private:
		// ── Layout ──────────────────────────────────────────────
		void DrawEditorDockspace();

		// ── Panels ──────────────────────────────────────────────
		void DrawViewport();          // centre – FBO image + toolbar strip
		void DrawHierarchy();
		void DrawObjectNode(GameObject* object);
		void DrawInspector();
		void DrawComponents(GameObject* object);
		void DrawContentBrowser();

		// ── Content browser helpers ──────────────────────────────
		void RefreshDirectory(const std::filesystem::path& dir);
		static const char* FileIcon(const std::filesystem::path& p);
		static bool IsImageFile(const std::filesystem::path& p);
		static bool IsMaterialFile(const std::filesystem::path& p);
		static bool IsSceneFile(const std::filesystem::path& p);
		static bool IsAudioFile(const std::filesystem::path& p);
		static bool IsMeshFile(const std::filesystem::path& p);

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

		// Dockspace – built once per activation
		bool m_dockspaceBuilt = false;
	};
}