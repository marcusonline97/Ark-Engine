#include "EditorUI.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <unordered_map>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "ImGuizmo/ImGuizmo.h"
#include "Input/Input.h"
#include "Input/KeyCodes.h"

#include "Rendering/Material/MtlMaterial.h"
#include "AssetManager.h"

#include <Utility/Utility.h>

namespace
{
    constexpr const char* kPayloadEditorObject = "ARK_EDITOR_OBJECT_ID";
    constexpr const char* kPayloadAssetPath = "ARK_EDITOR_ASSET_PATH";
}

std::uint32_t EditorUI::AllocateObjectId()
{
    return m_nextObjectId++;
}

void EditorUI::EnsureObjectIds(std::vector<EditorObject>& objects)
{
    std::uint32_t maxId = 0;
    for (auto& obj : objects)
    {
        if (obj.id == 0)
            obj.id = AllocateObjectId();
        maxId = std::max(maxId, obj.id);
    }
    if (m_nextObjectId <= maxId)
        m_nextObjectId = maxId + 1;
}

int EditorUI::FindObjectIndexById(const std::vector<EditorObject>& objects, std::uint32_t id) const
{
    if (id == 0) return -1;
    for (int i = 0; i < static_cast<int>(objects.size()); ++i)
    {
        if (objects[static_cast<size_t>(i)].id == id)
            return i;
    }
    return -1;
}

bool EditorUI::WouldCreateCycle(const std::vector<EditorObject>& objects, std::uint32_t childId, std::uint32_t newParentId) const
{
    if (childId == 0 || newParentId == 0) return false;
    if (childId == newParentId) return true;

    // Walk up from newParentId -> root; if we ever hit childId, we'd create a cycle.
    std::uint32_t cur = newParentId;
    for (int guard = 0; guard < 1024 && cur != 0; ++guard)
    {
        const int idx = FindObjectIndexById(objects, cur);
        if (idx < 0) break;
        const auto& obj = objects[static_cast<size_t>(idx)];
        if (obj.parentId == childId)
            return true;
        cur = obj.parentId;
    }
    return false;
}

Ark::Rendering::WorldCameraInput EditorUI::GetEditorViewportCamera() const
{
	const Ark::CameraController& activeCamera = m_isPossessing ? m_possessedCamera : m_editorCamera;

    Ark::Rendering::WorldCameraInput cam{};
	cam.position = activeCamera.position;

    // Editor camera is now also left-handed; no conversion here.
    cam.pitchYawDeg = glm::vec2(activeCamera.pitchDeg, activeCamera.yawDeg);

    cam.fovDeg = m_isPossessing ? m_possessedCamFovDeg : m_editorCamFovDeg;
    cam.nearPlane = m_isPossessing ? m_possessedCamNear : m_editorCamNear;
    cam.farPlane = m_isPossessing ? m_possessedCamFar : m_editorCamFar;
    return cam;
}

void EditorUI::ReparentObject(std::vector<EditorObject>& objects, std::uint32_t childId, std::uint32_t newParentId)
{
    const int childIdx = FindObjectIndexById(objects, childId);
    if (childIdx < 0) return;

    if (newParentId != 0 && FindObjectIndexById(objects, newParentId) < 0)
        newParentId = 0;

    if (WouldCreateCycle(objects, childId, newParentId))
        return;

    objects[static_cast<size_t>(childIdx)].parentId = newParentId;
}

std::string EditorUI::MakeProjectRelativePath(const std::filesystem::path& p) const
{
    if (p.empty())
        return {};

    std::error_code ec;
    const auto rel = std::filesystem::relative(p, m_projectRoot, ec);
    return ec ? p.string() : rel.string();
}


std::filesystem::path EditorUI::FindProjectRoot()
{
    // Prefer the directory that contains "ArkEngine/Resources"
    std::filesystem::path p = std::filesystem::current_path();
    while (true)
    {
        if (std::filesystem::exists(p / "ArkEngine" / "Resources"))
            return p;

        if (p == p.root_path())
            break;

        p = p.parent_path();
    }
    return std::filesystem::current_path();
}

std::filesystem::path EditorUI::FindResourcesRoot(const std::filesystem::path& projectRoot)
{
    const auto candidate = projectRoot / "ArkEngine" / "Resources";
    if (std::filesystem::exists(candidate))
        return candidate;
    return projectRoot;
}

void EditorUI::SetViewportCursorCapture(bool captured)
{
    if (m_viewportCursorCaptured == captured)
        return;

    if (!Ark::Input::g_Window)
        return;

    glfwSetInputMode(Ark::Input::g_Window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    m_viewportCursorCaptured = captured;

    double mx = 0.0;
    double my = 0.0;
    glfwGetCursorPos(Ark::Input::g_Window, &mx, &my);
    m_viewportLastMouse = ImVec2(static_cast<float>(mx), static_cast<float>(my));
}

bool EditorUI::BeginPossession(const std::vector<EditorObject>& objects, std::uint32_t targetObjectId)
{
    if (targetObjectId == 0)
        return false;

    const int targetIndex = FindObjectIndexById(objects, targetObjectId);
    if (targetIndex < 0)
        return false;

    const EditorObject& obj = objects[static_cast<size_t>(targetIndex)];
    if (!obj.enabled)
        return false;

    m_possessedObjectId = obj.id;
    m_possessedCamera.position = obj.position;
    m_possessedCamera.pitchDeg = obj.rotationDeg.x;
    m_possessedCamera.yawDeg = obj.rotationDeg.y;
    m_possessedCamFovDeg = obj.camera ? obj.camera->fovDeg : m_editorCamFovDeg;
    m_possessedCamNear = obj.camera ? obj.camera->nearPlane : m_editorCamNear;
    m_possessedCamFar = obj.camera ? obj.camera->farPlane : m_editorCamFar;
    m_isPossessing = true;
    m_viewportRmbLooking = false;
    SetViewportCursorCapture(true);
    return true;
}

void EditorUI::SyncPossessedObjectTransform(std::vector<EditorObject>& objects) const
{
    if (!m_isPossessing || m_possessedObjectId == 0)
        return;

    const int targetIndex = FindObjectIndexById(objects, m_possessedObjectId);
    if (targetIndex < 0)
        return;

    EditorObject& obj = objects[static_cast<size_t>(targetIndex)];
    obj.position = m_possessedCamera.position;
    obj.rotationDeg.x = m_possessedCamera.pitchDeg;
    obj.rotationDeg.y = m_possessedCamera.yawDeg;

    if (obj.camera)
    {
        obj.camera->fovDeg = m_possessedCamFovDeg;
        obj.camera->nearPlane = m_possessedCamNear;
        obj.camera->farPlane = m_possessedCamFar;
    }
}

void EditorUI::EndPossession()
{
    m_isPossessing = false;
    m_possessedObjectId = 0;
    m_viewportRmbLooking = false;
    SetViewportCursorCapture(false);
}


void EditorUI::Init()
{
    m_projectRoot = FindProjectRoot();
    m_resourcesRoot = FindResourcesRoot(m_projectRoot);
    m_contentDir = m_resourcesRoot;
    m_fileDir = m_projectRoot;

    m_dirScanner.Start();
    m_dirScanner.RequestScan(m_contentDir);
    m_dirScanner.RequestScan(m_fileDir);
    // Editor music player: expects <Resources>/Music/<Genre>/*.wav|*.mp3|...
    {
        ArkAudio::MusicPlayerConfig cfg;
        cfg.libraryRoot = m_resourcesRoot / "Music";
        cfg.preferredBackend = ArkAudio::MusicBackendType::Null; // will fall back if other backends are enabled at build-time
        cfg.shuffle = false;
        cfg.loopPlaylist = true;
        cfg.volume = m_musicVolume;
        m_music.Init(cfg);
    }

    Logging::EnableLevel(Logging::Level::INIT);
    Logging::EnableLevel(Logging::Level::DEBUG);
    Logging::EnableLevel(Logging::Level::WARNING);
    Logging::EnableLevel(Logging::Level::_ERROR);
    Logging::EnableLevel(Logging::Level::FATAL);
    Logging::EnableLevel(Logging::Level::TODO);
    Logging::EnableLevel(Logging::Level::FUNCTION);
}

void EditorUI::Shutdown()
{
    // nothing to add here since the panels are in intermediate mode
	EndPossession();
    m_music.Shutdown();
    m_dirScanner.Stop();
}


void EditorUI::PushLog(Logging::Level level, std::string_view msg)
{
    m_console.PushLog(level, msg);
}

void EditorUI::Render(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    ValidateSceneState(objects, selectedObjectIndex);

    RenderDockspace();

    if (m_showHierarchy)      RenderHierarchy(objects, selectedObjectIndex);
    if (m_showInspector)      RenderInspector(objects, selectedObjectIndex);
    if (m_showMaterials)      RenderMaterials(objects, selectedObjectIndex);
    if (m_showConsole)        RenderConsole();
    if (m_showContentBrowser) RenderContentBrowser();

    RenderViewport(objects, selectedObjectIndex);
    SyncPossessedObjectTransform(objects);

    if (m_showImGuiDemo)
        ImGui::ShowDemoWindow(&m_showImGuiDemo);

    ValidateSceneState(objects, selectedObjectIndex);
}

void EditorUI::RenderMaterials(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    if (!ImGui::Begin("Materials"))
    {
        ImGui::End();
        return;
    }

    if (selectedObjectIndex < 0 || selectedObjectIndex >= static_cast<int>(objects.size()))
    {
        ImGui::TextDisabled("Select an object to edit its material settings.");
        ImGui::End();
        return;
    }

    EditorObject& obj = objects[static_cast<size_t>(selectedObjectIndex)];

    ImGui::TextDisabled("Object: %s", obj.name.c_str());
    ImGui::Separator();

    ImGui::ColorEdit3("Tint", &obj.tint.x);

    static const char* kPresets[] =
    {
        "Default",
        "Matte",
        "Glossy",
        "Metal",
        "Emissive",
    };
    ImGui::Combo("Preset", &obj.materialPreset, kPresets, static_cast<int>(IM_ARRAYSIZE(kPresets)));

    ImGui::Separator();
    ImGui::TextDisabled("Component bindings (placeholder)");

    if (obj.staticMesh)
    {
        ImGui::TextUnformatted("Static Mesh");
        ImGui::InputText("Material", &obj.staticMesh->materialPath);
        ImGui::InputText("Texture", &obj.staticMesh->texturePath);
        ImGui::TextDisabled("Tip: drag an image into the Hierarchy to assign a texture.");
    }
    else if (obj.skeletalMesh)
    {
        ImGui::TextUnformatted("Skeletal Mesh");
        ImGui::InputText("Material", &obj.skeletalMesh->materialPath);
        ImGui::InputText("Texture", &obj.skeletalMesh->texturePath);
    }
    else
    {
        ImGui::TextDisabled("No renderable component on the selected object.");
    }

    ImGui::End();
}

void EditorUI::RenderConsole()
{
    // ConsolePanel owns the ImGui window and title.
    m_console.Render();
}

void EditorUI::RenderContentBrowser()
{
    if (!ImGui::Begin("Content Browser"))
    {
        ImGui::End();
        return;
    }

    DrawDirectoryBrowser("##ContentBrowserList", m_resourcesRoot, m_contentDir, &m_selectedAsset);
    ImGui::End();
}

void EditorUI::RenderFileExplorer()
{
    if (!ImGui::Begin("File Explorer"))
    {
        ImGui::End();
        return;
    }

    DrawDirectoryBrowser("##FileExplorerList", m_projectRoot, m_fileDir, &m_selectedFile);
    ImGui::End();
}

void EditorUI::DrawDirectoryBrowser(const char* id, const std::filesystem::path& root, std::filesystem::path& currentDir, std::filesystem::path* selectedPath)
{
    if (root.empty() || !std::filesystem::exists(root))
    {
        ImGui::TextDisabled("Root folder does not exist: %s", root.string().c_str());
        return;
    }

    // Keep the current directory valid and clamped to root.
    if (currentDir.empty() || !std::filesystem::exists(currentDir))
        currentDir = root;

    {
        std::error_code ec;
        const auto absRoot = std::filesystem::absolute(root, ec).lexically_normal();
        const auto absCur = std::filesystem::absolute(currentDir, ec).lexically_normal();
        if (!ec)
        {
            const auto rootStr = absRoot.string();
            const auto curStr = absCur.string();
            if (curStr.rfind(rootStr, 0) != 0) // not under root
                currentDir = root;
        }
    }

    if (ImGui::Button("Up"))
    {
        if (currentDir != root && currentDir.has_parent_path())
        {
            currentDir = currentDir.parent_path();
            m_dirScanner.RequestScan(currentDir);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh"))
        m_dirScanner.RequestScan(currentDir);

    ImGui::SameLine();
    ImGui::TextDisabled("%s", MakeProjectRelativePath(currentDir).c_str());

    ImGui::Separator();

    ImGui::BeginChild(id, ImVec2(0, 0), true);

    std::vector<Ark::Editor::DirectoryEntry> entries;
    if (!m_dirScanner.TryGetListing(currentDir, entries))
    {
        m_dirScanner.RequestScan(currentDir);
        ImGui::TextDisabled("Scanning...");
        ImGui::EndChild();
        return;
    }

    for (const auto& e : entries)
    {
        ImGui::PushID(e.name.c_str());

        const bool isSelected = (selectedPath && *selectedPath == e.path);
        const std::string label = e.isDirectory ? (std::string("[DIR] ") + e.name) : e.name;

        if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (e.isDirectory && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                currentDir = e.path;
                m_dirScanner.RequestScan(currentDir);
            }
            else if (!e.isDirectory && selectedPath)
            {
                *selectedPath = e.path;
            }
        }

        // Drag-drop assets into the hierarchy/inspector/etc.
        if (!e.isDirectory && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            const std::string rel = MakeProjectRelativePath(e.path);
            ImGui::SetDragDropPayload(kPayloadAssetPath, rel.c_str(), rel.size() + 1);
            ImGui::TextUnformatted(rel.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void EditorUI::RenderDockspace()
{
    static constexpr ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    RenderMenuBar();

    const ImGuiID dockspaceID = ImGui::GetID("ArkEditorDockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    EnsureDefaultLayout();

    ImGui::End();
}

void EditorUI::EnsureDefaultLayout()
{
    if (m_layoutBuilt) return;

    // If docking is disabled, don't attempt to build.
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
        return;

    const ImGuiID dockspaceID = ImGui::GetID("ArkEditorDockSpace");

    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->WorkSize);

    ImGuiID dockMain = dockspaceID;
    ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20f, nullptr, &dockMain);
    ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.28f, nullptr, &dockMain);
    ImGuiID dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.28f, nullptr, &dockMain);

    // Tabs on left and bottom, like your reference image.
    ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);

    ImGui::DockBuilderDockWindow("Inspector", dockRight);
    ImGui::DockBuilderDockWindow("Materials", dockRight);

    ImGui::DockBuilderDockWindow("Console", dockBottom);
    ImGui::DockBuilderDockWindow("Content Browser", dockBottom);

    ImGui::DockBuilderDockWindow("Viewport", dockMain);

    ImGui::DockBuilderFinish(dockspaceID);
    m_layoutBuilt = true;
}

void EditorUI::RenderMenuBar()
{
    if (!ImGui::BeginMenuBar())
        return;

    {
        ImGuiIO& io = ImGui::GetIO();
        const bool ctrl = io.KeyCtrl;

        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
            m_requestSaveScene = true;

        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
            m_requestLoadScene = true;
    }


    if (ImGui::BeginMenu("File"))
    {
        // inside EditorUI::RenderMenuBar(), in the "File" menu
        if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            m_requestSaveScene = true;

        if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            m_requestLoadScene = true;
        if (ImGui::MenuItem("Reset Layout"))
            m_layoutBuilt = false;
        if (ImGui::MenuItem("Quit"))
            Logging::ToDo() << "Quit action not wired yet.\n";
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window"))
    {
        ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchy);
        ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
        ImGui::MenuItem("Materials", nullptr, &m_showMaterials);

        ImGui::MenuItem("Console", nullptr, &m_showConsole);
        ImGui::MenuItem("Content Browser", nullptr, &m_showContentBrowser);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &m_showImGuiDemo);
        ImGui::EndMenu();
    }

    ImGui::TextDisabled("Project: %s", m_projectRoot.string().c_str());
    // Right-aligned viewport FPS.
    {
        const float fps = Utilities::GetViewportFPS();
        const uint32_t tris = m_viewportTriangleCount;

        char buf[96]{};
        std::snprintf(buf, sizeof(buf), "FPS: %.1f | Tris: %u", fps, tris);

        const float textWidth = ImGui::CalcTextSize(buf).x;
        const float avail = ImGui::GetContentRegionAvail().x;
        if (avail > textWidth + ImGui::GetStyle().ItemSpacing.x)
            ImGui::SameLine(ImGui::GetCursorPosX() + avail - textWidth);
        else
            ImGui::SameLine();
        ImGui::TextDisabled("%s", buf);
    }

    ImGui::EndMenuBar();
}


void EditorUI::RenderViewport(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    if (!ImGui::Begin("Viewport"))
    {
        ImGui::End();
        return;
    }

    const bool hasSelection = (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objects.size()));

    std::uint32_t possessCandidateId = 0;
    int possessCandidateScore = -1;
    for (size_t i = 0; i < objects.size(); ++i)
    {
        const EditorObject& candidate = objects[i];
        if (!candidate.enabled)
            continue;

        std::string loweredName = candidate.name;
        std::transform(
            loweredName.begin(),
            loweredName.end(),
            loweredName.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const bool isLikelyPlayer = loweredName.find("player") != std::string::npos;
        const bool isSelected = hasSelection && static_cast<int>(i) == selectedObjectIndex;

        int score = 0;
        if (isLikelyPlayer)
            score += 100;
        if (isSelected)
            score += 25;
        if (candidate.camera)
            score += 5;

        if (score > possessCandidateScore)
        {
            possessCandidateScore = score;
            possessCandidateId = candidate.id;
        }
    }
    const bool canPossess = possessCandidateId != 0;

    // Toolbar
    {
        const bool canEdit = !m_playMode;
        ImGui::TextDisabled("Gizmo:");
        ImGui::SameLine();

        ImGui::BeginDisabled(!canEdit || m_isPossessing);
        if (ImGui::SmallButton("Translate (W)")) m_gizmoMode = 0;
        ImGui::SameLine();
        if (ImGui::SmallButton("Rotate (E)")) m_gizmoMode = 1;
        ImGui::SameLine();
        if (ImGui::SmallButton("Scale (R)")) m_gizmoMode = 2;
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::SmallButton(m_playMode ? "Stop (F5)" : "Play (F5)"))
        {
            m_playMode = !m_playMode;
            if (m_playMode && m_isPossessing)
                EndPossession();
        }

        if (!m_playMode)
        {
            ImGui::SameLine();
            if (!m_isPossessing)
            {
                ImGui::BeginDisabled(!canPossess);
                if (ImGui::SmallButton("Possess Player (P)") && canPossess)
                    BeginPossession(objects, possessCandidateId);
                ImGui::EndDisabled();
            }
            else if (ImGui::SmallButton("Release (Esc)"))
            {
                EndPossession();
            }
        }

        ImGui::SameLine();
        ImGui::TextDisabled("Mode: %s", m_playMode ? "Play" : (m_isPossessing ? "Possess" : "Edit"));

        ImGui::SameLine();
        ImGui::Checkbox("Wireframe", &m_wireframeEnabled);
        ImGui::SameLine();
        ImGui::Checkbox("Mipmaps", &m_useMipmaps);
        ImGui::Separator();
    }

    const ImVec2 availableSize = ImGui::GetContentRegionAvail();
    m_viewportSize = glm::vec2(availableSize.x, availableSize.y);

    ImVec2 imageMin(0, 0);
    ImVec2 imageMax(0, 0);

    if (m_viewportTextureId != 0 && availableSize.x > 1.0f && availableSize.y > 1.0f)
    {
        imageMin = ImGui::GetCursorScreenPos();

        ImGui::Image(
            static_cast<ImTextureID>(m_viewportTextureId),
            availableSize,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );

        imageMax = ImGui::GetItemRectMax();

        const bool imgHovered = ImGui::IsItemHovered();
        ImGuiIO& io = ImGui::GetIO();

        if (imgHovered)
        {
            if (!m_isPossessing)
            {
                if (ImGui::IsKeyPressed(ImGuiKey_W)) m_gizmoMode = 0;
                if (ImGui::IsKeyPressed(ImGuiKey_E)) m_gizmoMode = 1;
                if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoMode = 2;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_F5))
            {
                m_playMode = !m_playMode;
                if (m_playMode && m_isPossessing)
                    EndPossession();
            }

            if (!m_playMode && !m_isPossessing && canPossess && ImGui::IsKeyPressed(ImGuiKey_P))
                BeginPossession(objects, possessCandidateId);
            if (m_isPossessing && ImGui::IsKeyPressed(ImGuiKey_Escape))
                EndPossession();

            if (ImGui::IsKeyPressed(ImGuiKey_G))
            {
                m_showGrid = !m_showGrid;
                Logging::Debug() << (m_showGrid ? "Grid enabled.\n" : "Grid cleared (hidden).\n");
            }
            if (ImGui::IsKeyPressed(ImGuiKey_H))
            {
                m_wireframeEnabled = !m_wireframeEnabled;
                Logging::Debug() << (m_wireframeEnabled ? "Wireframe enabled.\n" : "Wireframe disabled.\n");
            }
        }

        // EDIT/POSSESS camera controls (WASD + RMB look) only when hovered and not using gizmo.
        const bool allowViewportCameraInput = imgHovered && !m_playMode && !ImGuizmo::IsUsing();

        if (allowViewportCameraInput)
        {
            Ark::CameraController& activeCamera = m_isPossessing ? m_possessedCamera : m_editorCamera;

            Ark::CameraInput cameraInput{};
            cameraInput.forward = Ark::Input::IsKeyDown(ARK_KEY_W);
            cameraInput.back = Ark::Input::IsKeyDown(ARK_KEY_S);
            cameraInput.left = Ark::Input::IsKeyDown(ARK_KEY_A);
            cameraInput.right = Ark::Input::IsKeyDown(ARK_KEY_D);
            cameraInput.up = Ark::Input::IsKeyDown(ARK_KEY_E);
            cameraInput.down = Ark::Input::IsKeyDown(ARK_KEY_Q);
            cameraInput.fast = Ark::Input::IsKeyDown(ARK_KEY_LEFT_SHIFT);
            activeCamera.ProcessKeyboard(io.DeltaTime, cameraInput);

            const bool rmbDown = Ark::Input::IsMouseDown(ARK_MOUSE_RIGHT);
            if (rmbDown)
            {
                SetViewportCursorCapture(true);

                double mx = 0.0;
                double my = 0.0;
                glfwGetCursorPos(Ark::Input::g_Window, &mx, &my);

                const ImVec2 mousePos(static_cast<float>(mx), static_cast<float>(my));
                if (!m_viewportRmbLooking)
                {
                    m_viewportRmbLooking = true;
                    m_viewportLastMouse = mousePos;
                }
                else
                {
                    const float dx = mousePos.x - m_viewportLastMouse.x;
                    const float dy = mousePos.y - m_viewportLastMouse.y;
                    activeCamera.ProcessMouseDelta(dx, dy);
                    m_viewportLastMouse = mousePos;
                }
            }
            else
            {
                m_viewportRmbLooking = false;
                if (!m_isPossessing)
                    SetViewportCursorCapture(false);
            }
        }
        else
        {
            // Stop look state when leaving viewport / starting gizmo drag.
            m_viewportRmbLooking = false;
            if (!m_isPossessing)
                SetViewportCursorCapture(false);
        }

        if (!m_playMode && !m_isPossessing && hasSelection)
        {
            EditorObject& obj = objects[static_cast<size_t>(selectedObjectIndex)];
            const Ark::CameraController& activeCamera = m_editorCamera;

            // Use editor camera (NOT a fixed lookAt) for gizmo matrices
            const glm::vec3 camPos = activeCamera.position;
            const glm::vec3 camForward = activeCamera.GetForward();
            const glm::mat4 view = glm::lookAt(camPos, camPos + camForward, glm::vec3(0, 1, 0));

            const float aspect = (availableSize.y > 1.0f) ? (availableSize.x / availableSize.y) : 1.0f;
            const glm::mat4 projection = glm::perspective(glm::radians(m_editorCamFovDeg), aspect, m_editorCamNear, m_editorCamFar);

            glm::mat4 model(1.0f);
            {
                const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
                const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
                const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
                const glm::mat4 rot = rotZ * rotY * rotX;
                model = glm::translate(glm::mat4(1.0f), obj.position) * rot * glm::scale(glm::mat4(1.0f), obj.scale);
            }

            // Important: draw gizmo in viewport rect
            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
            ImGuizmo::SetRect(imageMin.x, imageMin.y, availableSize.x, availableSize.y);

            ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
            if (m_gizmoMode == 1) op = ImGuizmo::ROTATE;
            if (m_gizmoMode == 2) op = ImGuizmo::SCALE;

            ImGuizmo::MODE mode = (op == ImGuizmo::TRANSLATE) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

            glm::mat4 modelCopy = model;
            ImGuizmo::Manipulate(
                &view[0][0],
                &projection[0][0],
                op,
                mode,
                &modelCopy[0][0]);

            if (ImGuizmo::IsUsing())
            {
                float t[3]{}, r[3]{}, s[3]{};
                ImGuizmo::DecomposeMatrixToComponents(&modelCopy[0][0], t, r, s);
                obj.position = glm::vec3(t[0], t[1], t[2]);
                obj.rotationDeg = glm::vec3(r[0], r[1], r[2]);
                obj.scale = glm::vec3(s[0], s[1], s[2]);
            }
        }

        RenderViewportTextOverlays(objects, m_viewportRenderCamera, imageMin, availableSize);
    }
    else
    {
        ImGui::TextUnformatted("Viewport (waiting for scene render)");
        ImGui::Separator();
        ImGui::TextDisabled("Tip: the engine will render into a framebuffer and show it here.");
    }

    ImGui::End();
}

void EditorUI::RenderViewportTextOverlays(
    const std::vector<EditorObject>& objects,
    const Ark::Rendering::WorldCameraInput& camera,
    const ImVec2& imageMin,
    const ImVec2& viewportSize)
{
    if (viewportSize.x <= 1.0f || viewportSize.y <= 1.0f)
        return;

    const float aspect = viewportSize.x / viewportSize.y;
    const glm::mat4 projection = glm::perspectiveLH_ZO(
        glm::radians(camera.fovDeg),
        aspect,
        camera.nearPlane,
        camera.farPlane);

    const float pitchRad = glm::radians(camera.pitchYawDeg.x);
    const float yawRad = glm::radians(camera.pitchYawDeg.y);
    glm::vec3 forward;
    forward.x = std::cos(yawRad) * std::cos(pitchRad);
    forward.y = std::sin(pitchRad);
    forward.z = std::sin(yawRad) * std::cos(pitchRad);
    forward = glm::normalize(forward);

    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    const glm::vec3 right = glm::normalize(glm::cross(up, forward));
    const glm::mat4 view = glm::lookAtLH(camera.position, camera.position + forward, up);
    const glm::mat4 viewProj = projection * view;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (!drawList)
        return;

    for (const auto& obj : objects)
    {
        if (!obj.enabled || !obj.textRender || obj.textRender->text.empty())
            continue;

        const TextRenderEditorComponent& txt = *obj.textRender;

        glm::vec3 worldPos = obj.position + txt.worldOffset;
        if (txt.cameraBound)
        {
            worldPos = camera.position +
                right * txt.cameraOffset.x +
                up * txt.cameraOffset.y +
                forward * txt.cameraOffset.z;
        }

        const glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);
        if (clip.w <= 0.0001f)
            continue;

        const glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.z < 0.0f || ndc.z > 1.0f)
            continue;

        const float screenX = imageMin.x + ((ndc.x + 1.0f) * 0.5f) * viewportSize.x;
        const float screenY = imageMin.y + ((1.0f - ndc.y) * 0.5f) * viewportSize.y;

        const ImVec2 textSize = ImGui::CalcTextSize(txt.text.c_str());
        ImVec2 drawPos(screenX, screenY);
        if (txt.centerOnAnchor)
        {
            drawPos.x -= textSize.x * 0.5f;
            drawPos.y -= textSize.y * 0.5f;
        }

        const float alpha = std::clamp(txt.alpha, 0.0f, 1.0f);
        const ImU32 color = IM_COL32(
            static_cast<int>(std::clamp(txt.color.r, 0.0f, 1.0f) * 255.0f),
            static_cast<int>(std::clamp(txt.color.g, 0.0f, 1.0f) * 255.0f),
            static_cast<int>(std::clamp(txt.color.b, 0.0f, 1.0f) * 255.0f),
            static_cast<int>(alpha * 255.0f));

        drawList->AddText(
            ImGui::GetFont(),
            std::max(8.0f, txt.pixelSize),
            drawPos,
            color,
            txt.text.c_str());
    }
}

void EditorUI::RenderMusicPlayer()
{
    // Library controls
    const std::filesystem::path& root = m_music.GetLibraryRoot();
    const bool rootExists = std::filesystem::exists(root);

    ImGui::TextDisabled("Library: %s", root.string().c_str());

    if (!rootExists)
    {
        ImGui::TextWrapped("Create a folder structure like: Resources/Music/<Genre>/*.mp3");
        if (ImGui::Button("Create Resources/Music"))
        {
            std::error_code ec;
            std::filesystem::create_directories(root, ec);
            m_music.RescanLibrary();
        }
        ImGui::Separator();
    }

    if (ImGui::Button("Refresh Library"))
        m_music.RescanLibrary();

    ImGui::SameLine();
    bool shuffle = m_music.GetShuffle();
    if (ImGui::Checkbox("Shuffle", &shuffle))
        m_music.SetShuffle(shuffle);

    ImGui::SameLine();
    bool loop = m_music.GetLoopPlaylist();
    if (ImGui::Checkbox("Loop", &loop))
        m_music.SetLoopPlaylist(loop);

    ImGui::SameLine();
    if (ImGui::SliderFloat("Volume", &m_musicVolume, 0.0f, 1.0f, "%.2f"))
        m_music.SetVolume(m_musicVolume);

    ImGui::Separator();

    // Genre picker
    const auto& genres = m_music.GetGenres();
    const int selGenre = m_music.GetSelectedGenreIndex();
    const char* preview = (selGenre >= 0 && selGenre < static_cast<int>(genres.size()))
        ? genres[static_cast<size_t>(selGenre)].name.c_str()
        : "<none>";

    if (ImGui::BeginCombo("Genre", preview))
    {
        for (int i = 0; i < static_cast<int>(genres.size()); ++i)
        {
            const bool selected = (i == selGenre);
            if (ImGui::Selectable(genres[static_cast<size_t>(i)].name.c_str(), selected))
                m_music.SetSelectedGenreIndex(i);
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Track list
    const int trackCount = (selGenre >= 0 && selGenre < static_cast<int>(genres.size()))
        ? static_cast<int>(genres[static_cast<size_t>(selGenre)].tracks.size())
        : 0;

    ImGui::TextDisabled("Tracks: %d", trackCount);
    ImGui::BeginChild("##MusicTrackList", ImVec2(0, 220), true);
    if (selGenre >= 0 && selGenre < static_cast<int>(genres.size()))
    {
        const auto& tracks = genres[static_cast<size_t>(selGenre)].tracks;
        const int selTrack = m_music.GetSelectedTrackIndex();

        for (int i = 0; i < static_cast<int>(tracks.size()); ++i)
        {
            const bool selected = (i == selTrack);
            if (ImGui::Selectable(tracks[static_cast<size_t>(i)].name.c_str(), selected))
                m_music.SetSelectedTrackIndex(i);

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_music.SetSelectedTrackIndex(i);
                m_music.PlaySelected();
            }
        }
    }
    ImGui::EndChild();

    // Transport controls
    if (ImGui::Button("<<"))
        m_music.Previous();
    ImGui::SameLine();
    if (ImGui::Button("Play"))
        m_music.PlaySelected();
    ImGui::SameLine();
    if (ImGui::Button(m_music.IsPaused() ? "Resume" : "Pause"))
        m_music.TogglePause();
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        m_music.Stop();
    ImGui::SameLine();
    if (ImGui::Button(">>"))
        m_music.Next();

    ImGui::Separator();
    ImGui::TextUnformatted(m_music.GetStatusText().c_str());
}

void EditorUI::RenderHierarchy(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    if (!ImGui::Begin("Hierarchy"))
    {
        ImGui::End();
        return;
    }

    EnsureObjectIds(objects);

    const bool hasSelected = (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objects.size()));
    static bool parentToSelected = true;
    if (!hasSelected)
        parentToSelected = false;

    // F2 rename for the selected object when the hierarchy is focused.
    if (hasSelected && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_F2))
    {
        m_renamingObjectId = objects[static_cast<size_t>(selectedObjectIndex)].id;
        m_renameTarget = RenameTarget::Object;
        m_renameBuffer = objects[static_cast<size_t>(selectedObjectIndex)].name;
        m_focusRename = true;
    }

    const auto doCreate = [&](const char* defaultLabel, auto attachFn)
        {
            EditorObject obj{};
            obj.id = AllocateObjectId();
            obj.parentId = (parentToSelected && hasSelected) ? objects[static_cast<size_t>(selectedObjectIndex)].id : 0;
            obj.name = std::string(defaultLabel) + " " + std::to_string(objects.size() + 1);
            attachFn(obj);
            objects.push_back(std::move(obj));
            selectedObjectIndex = static_cast<int>(objects.size() - 1);
            Logging::Debug() << "Created GameObject.\n";
        };

    static constexpr const char* kCreatePopupId = "##HierarchyCreatePopup";

    if (ImGui::Button("Create"))
        ImGui::OpenPopup(kCreatePopupId);

    // Anchor the popup to the Create button (next to/under it).
    const ImVec2 createMin = ImGui::GetItemRectMin();
    const ImVec2 createMax = ImGui::GetItemRectMax();
    ImGui::SetNextWindowPos(ImVec2(createMin.x, createMax.y), ImGuiCond_Appearing);

    if (ImGui::BeginPopup(kCreatePopupId))
    {
        ImGui::TextDisabled("Create object");
        ImGui::Separator();

        ImGui::BeginDisabled(!hasSelected);
        ImGui::Checkbox("Parent to selected", &parentToSelected);
        ImGui::EndDisabled();
        ImGui::Separator();

        if (ImGui::MenuItem("Empty"))
            doCreate("GameObject", [](EditorObject&) {});

        ImGui::Separator();

        if (ImGui::BeginMenu("Meshes"))
        {
            if (ImGui::MenuItem("Static Mesh"))
                doCreate("StaticMesh", [](EditorObject& o) { o.staticMesh = StaticMeshEditorComponent{}; });
            if (ImGui::MenuItem("Skeletal Mesh"))
                doCreate("SkeletalMesh", [](EditorObject& o) { o.skeletalMesh = SkeletalMeshEditorComponent{}; });
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Camera"))
            doCreate("Camera", [](EditorObject& o) { o.camera = CameraEditorComponent{}; });

        if (ImGui::BeginMenu("Lights"))
        {
            if (ImGui::MenuItem("Point Light"))
                doCreate("PointLight", [](EditorObject& o) { o.pointLight = PointLightEditorComponent{}; });
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Text Render"))
            doCreate("Text", [](EditorObject& o) { o.textRender = TextRenderEditorComponent{}; });

        if (ImGui::MenuItem("Physics Body"))
            doCreate("PhysicsBody", [](EditorObject& o) { o.physicsBody = PhysicsBodyEditorComponent{}; });

        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete") && selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objects.size()))
    {
        const std::uint32_t deletedId = objects[static_cast<size_t>(selectedObjectIndex)].id;
        Logging::Warning() << "Deleted GameObject '" << objects[selectedObjectIndex].name << "'.\n";

        // Detach children of the deleted object to root.
        for (auto& o : objects)
        {
            if (o.parentId == deletedId)
                o.parentId = 0;
        }

        objects.erase(objects.begin() + selectedObjectIndex);
        selectedObjectIndex = objects.empty() ? -1 : std::min(selectedObjectIndex, static_cast<int>(objects.size() - 1));
    }

    ImGui::Separator();

    // Build children lists (id -> children ids).
    std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> childrenByParent;
    childrenByParent.reserve(objects.size());

    for (const auto& obj : objects)
    {
        const std::uint32_t pid = obj.parentId;
        if (pid != 0 && FindObjectIndexById(objects, pid) >= 0)
            childrenByParent[pid].push_back(obj.id);
    }
    std::vector<std::uint32_t> roots;
    roots.reserve(objects.size());
    for (const auto& obj : objects)
    {
        if (obj.parentId == 0 || FindObjectIndexById(objects, obj.parentId) < 0)
            roots.push_back(obj.id);
    }

    const auto isImage = [](const std::filesystem::path& p)
        {
            const auto ext = p.extension().string();
            auto e = ext;
            std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return (e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".bmp" || e == ".tga" || e == ".dds");
        };
    const auto isMesh = [](const std::filesystem::path& p)
        {
            const auto ext = p.extension().string();
            auto e = ext;
            std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return (e == ".fbx" || e == ".obj" || e == ".gltf" || e == ".glb" || e == ".dae");
        };

    const auto isAnim = [](const std::filesystem::path& p)
        {
            const auto ext = p.extension().string();
            auto e = ext;
            std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return (e == ".fbx" || e == ".dae" || e == ".gltf" || e == ".glb");
        };
    // Recursive tree draw.
    std::function<void(std::uint32_t)> drawNode = [&](std::uint32_t id)
        {
            const int idx = FindObjectIndexById(objects, id);
            if (idx < 0) return;
            EditorObject& obj = objects[static_cast<size_t>(idx)];

            ImGui::PushID(static_cast<int>(obj.id));
            const bool selected = (selectedObjectIndex == idx);
            const bool hasChildren = (childrenByParent.find(obj.id) != childrenByParent.end() && !childrenByParent[obj.id].empty());

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                (selected ? ImGuiTreeNodeFlags_Selected : 0) |
                (!hasChildren ? ImGuiTreeNodeFlags_Leaf : 0);
            const bool renamingThisObject = (m_renamingObjectId == obj.id && m_renameTarget == RenameTarget::Object);
            bool open = false;
            if (!renamingThisObject)
            {
                open = ImGui::TreeNodeEx("##obj", flags, "%s", obj.name.c_str());
            }
            else
            {
                open = ImGui::TreeNodeEx("##obj", flags, "%s", "");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1.0f);
                if (m_focusRename)
                {
                    ImGui::SetKeyboardFocusHere();
                    m_focusRename = false;
                }
                const bool committed = ImGui::InputText("##rename_obj", &m_renameBuffer, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
                const bool canceled = ImGui::IsKeyPressed(ImGuiKey_Escape);
                if (committed || (!ImGui::IsItemActive() && ImGui::IsItemDeactivatedAfterEdit()))
                {
                    if (!m_renameBuffer.empty())
                        obj.name = m_renameBuffer;
                    m_renamingObjectId = 0;
                    m_renameTarget = RenameTarget::None;
                }
                else if (canceled)
                {
                    m_renamingObjectId = 0;
                    m_renameTarget = RenameTarget::None;
                }
            }

            if (ImGui::IsItemClicked())
                selectedObjectIndex = idx;

            // Double-click rename on objects
            if (!renamingThisObject && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                m_renamingObjectId = obj.id;
                m_renameTarget = RenameTarget::Object;
                m_renameBuffer = obj.name;
                m_focusRename = true;
            }

            // Drag this object.
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload(kPayloadEditorObject, &obj.id, sizeof(obj.id));
                ImGui::Text("Attach: %s", obj.name.c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target: parenting or asset assignment.
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadEditorObject))
                {
                    const std::uint32_t childId = *static_cast<const std::uint32_t*>(payload->Data);
                    ReparentObject(objects, childId, obj.id);
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
                {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    if (dropped && dropped[0] != '\0')
                    {
                        const std::filesystem::path assetPath(dropped);
                        if (isMesh(assetPath))
                        {
                            if (!obj.staticMesh && !obj.skeletalMesh)
                                obj.staticMesh = StaticMeshEditorComponent{};

                            if (obj.staticMesh)
                                obj.staticMesh->meshPath = dropped;
                            else if (obj.skeletalMesh)
                                obj.skeletalMesh->meshPath = dropped;
                        }
                        else if (isImage(assetPath))
                        {
                            if (!obj.staticMesh)
                                obj.staticMesh = StaticMeshEditorComponent{};
                            obj.staticMesh->texturePath = dropped;
                        }
                        else if (isAnim(assetPath))
                        {
                            if (!obj.skeletalMesh)
                                obj.skeletalMesh = SkeletalMeshEditorComponent{};
                            obj.skeletalMesh->animationPath = dropped;
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Context menu for components / detach.
            if (ImGui::BeginPopupContextItem("##obj_ctx"))
            {
                if (ImGui::MenuItem("Detach from parent", nullptr, false, obj.parentId != 0))
                    obj.parentId = 0;

                if (ImGui::MenuItem("Rename", "F2"))
                {
                    m_renamingObjectId = obj.id;
                    m_renameTarget = RenameTarget::Object;
                    m_renameBuffer = obj.name;
                    m_focusRename = true;
                }

                if (ImGui::BeginMenu("Add Component"))
                {
                    if (!obj.staticMesh && ImGui::MenuItem("Static Mesh")) obj.staticMesh = StaticMeshEditorComponent{};
                    if (!obj.skeletalMesh && ImGui::MenuItem("Skeletal Mesh")) obj.skeletalMesh = SkeletalMeshEditorComponent{};
                    if (!obj.camera && ImGui::MenuItem("Camera")) obj.camera = CameraEditorComponent{};
                    if (!obj.pointLight && ImGui::MenuItem("Point Light")) obj.pointLight = PointLightEditorComponent{};
                    if (!obj.textRender && ImGui::MenuItem("Text Render")) obj.textRender = TextRenderEditorComponent{};
                    if (!obj.physicsBody && ImGui::MenuItem("Physics Body")) obj.physicsBody = PhysicsBodyEditorComponent{};
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Remove Component"))
                {
                    if (obj.staticMesh && ImGui::MenuItem("Static Mesh")) obj.staticMesh.reset();
                    if (obj.skeletalMesh && ImGui::MenuItem("Skeletal Mesh")) obj.skeletalMesh.reset();
                    if (obj.camera && ImGui::MenuItem("Camera")) obj.camera.reset();
                    if (obj.pointLight && ImGui::MenuItem("Point Light")) obj.pointLight.reset();
                    if (obj.textRender && ImGui::MenuItem("Text Render")) obj.textRender.reset();
                    if (obj.physicsBody && ImGui::MenuItem("Physics Body")) obj.physicsBody.reset();
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }

            if (open)
            {
                // Hierarchy shows only object nodes (no component/property editing here).
                // Component editing belongs in the Inspector.

                // Children
                const auto it = childrenByParent.find(obj.id);
                if (it != childrenByParent.end())
                {
                    for (const auto child : it->second)
                        drawNode(child);
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        };

    for (const auto rootId : roots)
        drawNode(rootId);

    // Dropping an object onto empty hierarchy detaches it to root.
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadEditorObject))
        {
            const std::uint32_t childId = *static_cast<const std::uint32_t*>(payload->Data);
            ReparentObject(objects, childId, 0);
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
        {
            const char* dropped = static_cast<const char*>(payload->Data);
            if (dropped && dropped[0] != '\0')
            {
                const std::filesystem::path assetPath(dropped);
                if (isMesh(assetPath))
                {
                    EditorObject obj{};
                    obj.id = AllocateObjectId();
                    obj.parentId = 0;
                    obj.name = assetPath.stem().empty() ? "StaticMesh" : assetPath.stem().string();
                    obj.staticMesh = StaticMeshEditorComponent{};
                    obj.staticMesh->meshPath = dropped;
                    objects.push_back(std::move(obj));
                    selectedObjectIndex = static_cast<int>(objects.size() - 1);
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
    ImGui::End();
}
bool EditorUI::ConsumeSaveSceneRequested()
{
    const bool v = m_requestSaveScene;
    m_requestSaveScene = false;
    return v;
}

bool EditorUI::ConsumeLoadSceneRequested()
{
    const bool v = m_requestLoadScene;
    m_requestLoadScene = false;
    return v;
}

void EditorUI::RenderInspector(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    if (!ImGui::Begin("Inspector"))
    {
        ImGui::End();
        return;
    }

    if (selectedObjectIndex < 0 || selectedObjectIndex >= static_cast<int>(objects.size()))
    {
        ImGui::TextDisabled("Select an object from the Hierarchy.");
        ImGui::End();
        return;
    }

    EditorObject& obj = objects[static_cast<size_t>(selectedObjectIndex)];

    // Header
    ImGui::Checkbox("Enabled", &obj.enabled);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##ObjectName", &obj.name);

    // Transform
    ImGui::Separator();
    ImGui::TextUnformatted("Transform");

    if (ImGui::Button("Reset##TransformReset"))
    {
        obj.position = glm::vec3(0.0f);
        obj.rotationDeg = glm::vec3(0.0f);
        obj.scale = glm::vec3(1.0f);
    }

    ImGui::DragFloat3("Position", &obj.position.x, 0.05f);
    ImGui::DragFloat3("Rotation (deg)", &obj.rotationDeg.x, 0.25f);
    ImGui::DragFloat3("Scale", &obj.scale.x, 0.05f, 0.001f, 1000.0f);

    // Material
    ImGui::Separator();
    ImGui::TextUnformatted("Material");
    ImGui::ColorEdit3("Tint", &obj.tint.x);

    static const char* kPresets[] =
    {
        "Default",
        "Matte",
        "Glossy",
        "Metal",
        "Emissive",
    };
    ImGui::Combo("Preset", &obj.materialPreset, kPresets, static_cast<int>(IM_ARRAYSIZE(kPresets)));

    // Mesh / asset bindings
    ImGui::Separator();
    ImGui::TextUnformatted("Mesh / Assets");

    const auto applyDroppedAsset = [&](const char* dropped)
        {
            if (!dropped || dropped[0] == '\0')
                return;

            const std::filesystem::path p(dropped);
            std::string ext = p.has_extension() ? p.extension().string() : std::string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            const bool isMesh = (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".dae");
            const bool isImage = (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".dds");

            if (isMesh)
            {
                if (!obj.staticMesh && !obj.skeletalMesh)
                    obj.staticMesh = StaticMeshEditorComponent{};

                if (obj.staticMesh)
                    obj.staticMesh->meshPath = dropped;
                else if (obj.skeletalMesh)
                    obj.skeletalMesh->meshPath = dropped;
            }
            else if (isImage)
            {
                if (!obj.staticMesh && !obj.skeletalMesh)
                    obj.staticMesh = StaticMeshEditorComponent{};

                if (obj.staticMesh)
                    obj.staticMesh->texturePath = dropped;
                else if (obj.skeletalMesh)
                    obj.skeletalMesh->texturePath = dropped;
            }
        };

    const auto acceptMeshDropToString = [&](std::string& dst)
        {
            if (!ImGui::BeginDragDropTarget())
                return;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
            {
                const char* dropped = static_cast<const char*>(payload->Data);
                if (dropped && dropped[0] != '\0')
                {
                    const std::filesystem::path p(dropped);
                    std::string ext = p.has_extension() ? p.extension().string() : std::string();
                    std::transform(ext.begin(), ext.end(), ext.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    const bool isMesh = (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb" || ext == ".dae");
                    if (isMesh)
                        dst = dropped;
                }
            }

            ImGui::EndDragDropTarget();
        };

    const auto acceptImageDropToString = [&](std::string& dst)
        {
            if (!ImGui::BeginDragDropTarget())
                return;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
            {
                const char* dropped = static_cast<const char*>(payload->Data);
                if (dropped && dropped[0] != '\0')
                {
                    const std::filesystem::path p(dropped);
                    std::string ext = p.has_extension() ? p.extension().string() : std::string();
                    std::transform(ext.begin(), ext.end(), ext.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    const bool isImage = (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".dds");
                    if (isImage)
                        dst = dropped;
                }
            }

            ImGui::EndDragDropTarget();
        };

    if (obj.staticMesh)
    {
        if (ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputText("Mesh Path", &obj.staticMesh->meshPath);
            acceptMeshDropToString(obj.staticMesh->meshPath);

            ImGui::InputText("Material Path (.mtl)", &obj.staticMesh->materialPath);

            auto acceptMtlDropToString = [&](std::string& dst)
            {
                if (!ImGui::BeginDragDropTarget())
                    return false;

                bool changed = false;

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
                {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    if (dropped && dropped[0] != '\0')
                    {
                        const std::filesystem::path p(dropped);
                        std::string ext = p.has_extension() ? p.extension().string() : std::string();
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (ext == ".mtl")
                        {
                            dst = dropped;
                            changed = true;
                        }
                    }
                }

                ImGui::EndDragDropTarget();
                return changed;
            };

            const auto rebuildMtlTextureSlots = [&]()
            {
                obj.staticMesh->textures.clear();

                if (obj.staticMesh->materialPath.empty())
                    return;

                const std::string resolved = AssetManager::Instance().ResolveAssetPath(obj.staticMesh->materialPath);

                Ark::Rendering::MtlMaterial mtl{};
                if (!Ark::Rendering::TryLoadMtlMaterial(resolved, mtl))
                    return;

                const auto addAll = [&](const char* name, Ark::Rendering::MtlTextureSemantic sem)
                {
                    auto it = mtl.textures.find(sem);
                    if (it == mtl.textures.end() || it->second.empty())
                        return;

                    for (const auto& path : it->second)
                    {
                        if (path.empty())
                            continue;

                        StaticMeshEditorComponent::TextureSlot slot{};
                        slot.name = name;
                        slot.path = path;
                        obj.staticMesh->textures.push_back(std::move(slot));
                    }
                };

                addAll("Diffuse", Ark::Rendering::MtlTextureSemantic::Diffuse);
                addAll("Specular", Ark::Rendering::MtlTextureSemantic::Specular);
                addAll("Normal", Ark::Rendering::MtlTextureSemantic::Normal);
                addAll("Opacity", Ark::Rendering::MtlTextureSemantic::Opacity);
            };

            const bool mtlDropped = acceptMtlDropToString(obj.staticMesh->materialPath);

            // If user typed a new value or dropped a new .mtl, rebuild slots automatically.
            if (mtlDropped || ImGui::IsItemDeactivatedAfterEdit())
                rebuildMtlTextureSlots();

            ImGui::SameLine();
            if (ImGui::Button("Parse .mtl"))
                rebuildMtlTextureSlots();

            if (!obj.staticMesh->textures.empty())
            {
                ImGui::SeparatorText("MTL Textures");
                for (size_t i = 0; i < obj.staticMesh->textures.size(); ++i)
                {
                    auto& s = obj.staticMesh->textures[i];
                    ImGui::PushID(static_cast<int>(i));
                    ImGui::InputText("Name", &s.name);
                    ImGui::InputText("Path", &s.path);
                    ImGui::PopID();
                }
            }

            ImGui::InputText("Texture", &obj.staticMesh->texturePath);
            acceptImageDropToString(obj.staticMesh->texturePath);

            ImGui::TextDisabled("Tip: drag-drop a mesh/texture from Content Browser onto this section or the Mesh/Texture Path fields.");

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
                {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    applyDroppedAsset(dropped);
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::Button("Remove Static Mesh"))
                obj.staticMesh.reset();
        }
    }
    else if (obj.skeletalMesh)
    {
        if (ImGui::CollapsingHeader("Skeletal Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputText("Mesh Path", &obj.skeletalMesh->meshPath);
            acceptMeshDropToString(obj.skeletalMesh->meshPath);

            ImGui::InputText("Animation Path", &obj.skeletalMesh->animationPath);
            ImGui::DragInt("Anim Index", &obj.skeletalMesh->animationIndex, 1.0f, -1, 1024);
            ImGui::InputText("Material Path", &obj.skeletalMesh->materialPath);
            ImGui::InputText("Texture Path", &obj.skeletalMesh->texturePath);

            ImGui::TextDisabled("Tip: drag-drop a mesh/texture from Content Browser onto this section or the Mesh Path field.");

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAssetPath))
                {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    applyDroppedAsset(dropped);
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::Button("Remove Skeletal Mesh"))
                obj.skeletalMesh.reset();
        }
    }
    else
    {
        ImGui::TextDisabled("No mesh component. Add one from the Hierarchy context menu.");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Text Render");
    if (obj.textRender)
    {
        ImGui::InputText("Text", &obj.textRender->text);
        ImGui::ColorEdit3("Text Color", &obj.textRender->color.x);
        ImGui::SliderFloat("Text Alpha", &obj.textRender->alpha, 0.0f, 1.0f, "%.2f");
        ImGui::DragFloat("Pixel Size", &obj.textRender->pixelSize, 0.25f, 8.0f, 96.0f, "%.1f");
        ImGui::Checkbox("Center On Anchor", &obj.textRender->centerOnAnchor);
        ImGui::Checkbox("Camera Bound", &obj.textRender->cameraBound);
        if (obj.textRender->cameraBound)
        {
            ImGui::DragFloat3("Camera Offset", &obj.textRender->cameraOffset.x, 0.02f);
            ImGui::TextDisabled("Offset axes are camera-right / camera-up / camera-forward.");
        }
        else
        {
            ImGui::DragFloat3("World Offset", &obj.textRender->worldOffset.x, 0.02f);
            ImGui::TextDisabled("World text anchor = object position + world offset.");
        }

        if (ImGui::Button("Remove Text Render"))
            obj.textRender.reset();
    }
    else
    {
        ImGui::TextDisabled("No text component.");
        if (ImGui::Button("Add Text Render"))
            obj.textRender = TextRenderEditorComponent{};
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Physics");
    if (obj.physicsBody)
    {
        static const char* kMotionTypes[] =
        {
            "Static",
            "Dynamic",
            "Kinematic",
        };

        ImGui::Combo("Motion Type", &obj.physicsBody->motionType, kMotionTypes, static_cast<int>(IM_ARRAYSIZE(kMotionTypes)));
        ImGui::Checkbox("Use Gravity", &obj.physicsBody->useGravity);
        ImGui::DragFloat3("Half Extents", &obj.physicsBody->halfExtents.x, 0.02f, 0.01f, 100.0f, "%.2f");

        if (ImGui::Button("Remove Physics Body"))
            obj.physicsBody.reset();
    }
    else
    {
        ImGui::TextDisabled("No physics body component.");
        if (ImGui::Button("Add Physics Body"))
            obj.physicsBody = PhysicsBodyEditorComponent{};
    }

    ImGui::End();
}
void EditorUI::ValidateSceneState(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    EnsureObjectIds(objects);

    if (objects.empty())
    {
        selectedObjectIndex = -1;
        return;
    }

    if (selectedObjectIndex < -1)
        selectedObjectIndex = -1;

    if (selectedObjectIndex >= static_cast<int>(objects.size()))
        selectedObjectIndex = static_cast<int>(objects.size() - 1);
}