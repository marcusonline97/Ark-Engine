#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "Logger.h"
#include "MusicPlayer/MusicPlayer.h"
#include "Editor/DirectoryScanner.h"
#include "Panels/ConsolePanel.h"

struct StaticMeshEditorComponent
{
    // .fbx/.obj (relative to project root / Resources layout)
    std::string meshPath;

    // Placeholder material bindings (the runtime material system is still evolving)
    std::string materialPath;
    std::string texturePath;
};

struct SkeletalMeshEditorComponent
{
    // .fbx/.obj with armature/skin data
    std::string meshPath;

    // Optional animation source
    std::string animationPath;
    int animationIndex = -1;

    std::string materialPath;
    std::string texturePath;
};

struct CameraEditorComponent
{
    bool primary = true;
    float fovDeg = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

struct PointLightEditorComponent
{
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float radius = 1.0f;
};

struct EditorObject
{
    std::string name = "GameObject";
    bool enabled = true;
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotationDeg{ 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

	glm::vec3 tint{ 1.0f, 1.0f, 1.0f };
    int materialPreset = 0;

    // Optional components (until the real Scene/ECS is wired in)
    std::optional<StaticMeshEditorComponent> staticMesh;
    std::optional<SkeletalMeshEditorComponent> skeletalMesh;
    std::optional<CameraEditorComponent> camera;
    std::optional<PointLightEditorComponent> pointLight;
};

class EditorUI
{
public:
    void Init();
    void Shutdown();

    void Render(std::vector<EditorObject>& objects, int& selectedObjectIndex);

    void SetViewportTextureId(unsigned int textureId) { m_viewportTextureId = textureId; }
    unsigned int GetViewportTextureId() const { return m_viewportTextureId; }
    glm::vec2 GetViewportSize() const { return m_viewportSize; }

    bool IsPlayMode() const { return m_playMode; }

    // a Thread-safe entry point for my Logging sink callback
    void PushLog(Logging::Level level, std::string_view msg);

private:


    void RenderDockspace();
    void RenderMenuBar();

    void RenderViewport();
	void RenderMusicPlayer();
    void RenderHierarchy(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void RenderInspector(std::vector<EditorObject>& objects, int& selectedObjectIndex);

	void RenderMaterials(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void RenderConsole();
    void RenderContentBrowser();
    void RenderFileExplorer();

    void EnsureDefaultLayout();

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

    bool m_playMode = false;

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
};