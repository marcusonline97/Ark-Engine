#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <entt.hpp>

#include "Logger.h"
#include "MusicPlayer/MusicPlayer.h"
#include "Editor/DirectoryScanner.h"
#include "Panels/ConsolePanel.h"

namespace Ark { class Scene; }

class EditorUI
{
public:
    void Init();
    void Shutdown();

    void Render(Ark::Scene& scene, entt::entity& selectedEntity);

    void SetViewportTextureId(unsigned int textureId) { m_viewportTextureId = textureId; }
    unsigned int GetViewportTextureId() const { return m_viewportTextureId; }
    glm::vec2 GetViewportSize() const { return m_viewportSize; }

    // a Thread-safe entry point for my Logging sink callback
    void PushLog(Logging::Level level, std::string_view msg);

private:


    void RenderDockspace();
    void RenderMenuBar();

    void RenderViewport(Ark::Scene& scene, entt::entity selectedEntity);
	void RenderMusicPlayer();
    void RenderHierarchy(Ark::Scene& scene, entt::entity& selectedEntity);
    void RenderInspector(Ark::Scene& scene, entt::entity& selectedEntity);

	void RenderMaterials(Ark::Scene& scene, entt::entity& selectedEntity);
    void RenderConsole();
    void RenderContentBrowser();
    void RenderFileExplorer();

    void EnsureDefaultLayout();

    void RenderEntityTree(Ark::Scene& scene, entt::entity entity, entt::entity& selectedEntity);
    void RenderGizmo(Ark::Scene& scene, entt::entity selectedEntity, const ImVec2& viewportMin, const ImVec2& viewportSize);

    // Shared directory browser helper
    void DrawDirectoryBrowser(const char* id, const std::filesystem::path& root, std::filesystem::path& currentDir, std::filesystem::path* selectedPath);

    static std::filesystem::path FindProjectRoot();
    static std::filesystem::path FindResourcesRoot(const std::filesystem::path& projectRoot);

private:
    bool m_layoutBuilt = false;

    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showMaterials = true;

    bool m_showConsole = true;
    bool m_showContentBrowser = true;
    bool m_showFileExplorer = true;
    bool m_showImGuiDemo = false;

    std::filesystem::path m_projectRoot;
    std::filesystem::path m_resourcesRoot;
    std::filesystem::path m_contentDir;
    std::filesystem::path m_fileDir;

    std::filesystem::path m_selectedAsset;
    std::filesystem::path m_selectedFile;

    Ark::Editor::ConsolePanel m_console;
    Ark::Editor::DirectoryScanner m_dirScanner;


	ArkAudio::MusicPlayer m_music;
	float m_musicVolume = 0.5f;


    // Viewport  
    unsigned int m_viewportTextureId = 0;
    glm::vec2 m_viewportSize{ 0.0f, 0.0f };

    // Gizmo
    int m_gizmoOperation = 0; // ImGuizmo::TRANSLATE by default
    int m_gizmoMode = 1;      // ImGuizmo::WORLD by default
    bool m_gizmoEnabled = true;
};