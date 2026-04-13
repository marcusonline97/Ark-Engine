#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <glm/glm.hpp>

#include "Camera/CameraController.h"
#include "Logger.h"
#include "MusicPlayer/MusicPlayer.h"
#include "Editor/DirectoryScanner.h"
#include "Panels/ConsolePanel.h"

#include "Rendering/Threading/WorldRenderThread.h"

struct StaticMeshEditorComponent
{
    struct TextureSlot
    {
        std::string name; // e.g. "Diffuse", "Normal"
        std::string path; // project-relative
    };

    std::string meshPath;
    std::string displayName = "Static Mesh";

    // For OBJ: user can specify an explicit .mtl to override/augment imported material info.
    std::string materialPath;

    // Back-compatability: used as "albedo override" and for older scenes/UI.
    std::string texturePath;

    // Generated from .mtl
    std::vector<TextureSlot> textures;
};

struct SkeletalMeshEditorComponent
{
    std::string meshPath;
    std::string displayName = "Skeletal Mesh";
    std::string animationPath;
    int animationIndex = -1;
    std::string materialPath;
    std::string texturePath;
};

struct CameraEditorComponent
{
    std::string displayName = "Camera";
    bool primary = true;
    float fovDeg = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

struct PointLightEditorComponent
{
    std::string displayName = "Point Light";
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float radius = 1.0f;
};

struct EditorObject
{
    std::uint32_t id = 0;
    std::uint32_t parentId = 0;

    std::string name = "GameObject";
    bool enabled = true;
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotationDeg{ 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    glm::vec3 tint{ 1.0f, 1.0f, 1.0f };
    int materialPreset = 0;

    bool flipUvV = false;

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

    Ark::Rendering::WorldCameraInput GetEditorViewportCamera() const;

    bool GetWireframeEnabled() const { return m_wireframeEnabled; }

    bool GetUseMipmaps() const { return m_useMipmaps; }

    void PushLog(Logging::Level level, std::string_view msg);

    bool ConsumeSaveSceneRequested();
    bool ConsumeLoadSceneRequested();

    bool GetShowGrid() const { return m_showGrid; }

    void SetViewportTriangleCount(uint32_t tris) { m_viewportTriangleCount = tris; }
    uint32_t GetViewportTriangleCount() const { return m_viewportTriangleCount; }

private:
    void RenderDockspace();
    void RenderMenuBar();

    void RenderViewport(std::vector<EditorObject>& objects, int& selectedObjectIndex);

    void RenderMusicPlayer();

    void RenderHierarchy(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void RenderInspector(std::vector<EditorObject>& objects, int& selectedObjectIndex);

    void RenderMaterials(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void RenderConsole();
    void RenderContentBrowser();
    void RenderFileExplorer();

    void EnsureDefaultLayout();

    void DrawDirectoryBrowser(const char* id, const std::filesystem::path& root, std::filesystem::path& currentDir, std::filesystem::path* selectedPath);

    static std::filesystem::path FindProjectRoot();
    static std::filesystem::path FindResourcesRoot(const std::filesystem::path& projectRoot);

    std::uint32_t AllocateObjectId();
    void EnsureObjectIds(std::vector<EditorObject>& objects);
    int FindObjectIndexById(const std::vector<EditorObject>& objects, std::uint32_t id) const;
    bool WouldCreateCycle(const std::vector<EditorObject>& objects, std::uint32_t childId, std::uint32_t newParentId) const;
    void ReparentObject(std::vector<EditorObject>& objects, std::uint32_t childId, std::uint32_t newParentId);
    std::string MakeProjectRelativePath(const std::filesystem::path& p) const;

    void ValidateSceneState(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void SetViewportCursorCapture(bool captured);
    bool BeginPossession(const std::vector<EditorObject>& objects, std::uint32_t targetObjectId);
    void EndPossession();

private:
    bool m_layoutBuilt = false;

    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showMaterials = true;
    bool m_wireframeEnabled = false;
    bool m_useMipmaps = true;

    bool m_showConsole = true;
    bool m_showContentBrowser = true;
    bool m_showFileExplorer = true;
    bool m_showImGuiDemo = false;
    bool m_showGrid = true;

    bool m_playMode = false;
    bool m_requestSaveScene = false;
    bool m_requestLoadScene = false;

    int m_gizmoMode = 0;

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

    std::uint32_t m_nextObjectId = 1;

    std::uint32_t m_renamingObjectId = 0;
    enum class RenameTarget { None, Object, StaticMesh, SkeletalMesh, Camera, PointLight };
    RenameTarget m_renameTarget = RenameTarget::None;
    std::string m_renameBuffer;
    bool m_focusRename = false;

    // Viewport
    unsigned int m_viewportTextureId = 0;
    glm::vec2 m_viewportSize{ 0.0f, 0.0f };

    // Editor/possessed viewport camera state (used in EDIT mode)
    Ark::CameraController m_editorCamera{ { 0.0f, 0.0f, 3.0f }, 0.0f, -90.0f, 3.5f, 0.12f };
    Ark::CameraController m_possessedCamera{ { 0.0f, 0.0f, 3.0f }, 0.0f, -90.0f, 3.5f, 0.12f };
    float m_editorCamFovDeg = 45.0f;
    float m_editorCamNear = 0.1f;
    float m_editorCamFar = 100.0f;
    bool m_isPossessing = false;
    std::uint32_t m_possessedObjectId = 0;

    // Input bookkeeping for viewport look/capture
    bool m_viewportRmbLooking = false;
    bool m_viewportCursorCaptured = false;
    ImVec2 m_viewportLastMouse{ 0.0f, 0.0f };

    uint32_t m_viewportTriangleCount = 0;
};