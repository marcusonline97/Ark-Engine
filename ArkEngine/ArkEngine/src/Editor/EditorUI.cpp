#include "EditorUI.h"

#include <algorithm>
#include <cstdio>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Utility/Utility.h>


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
	m_music.Shutdown();
    m_dirScanner.Stop();

}


void EditorUI::PushLog(Logging::Level level, std::string_view msg)
{
    m_console.PushLog(level, msg);

}

void EditorUI::Render(std::vector<EditorObject>& objects, int& selectedObjectIndex)
{
    RenderDockspace();

    if (m_showHierarchy)      RenderHierarchy(objects, selectedObjectIndex);
    if (m_showInspector)      RenderInspector(objects, selectedObjectIndex);
    if (m_showMaterials)      RenderMaterials(objects, selectedObjectIndex);
    if (m_showConsole)        RenderConsole();
    if (m_showContentBrowser) RenderContentBrowser();
    if (m_showFileExplorer)   RenderFileExplorer();

    RenderViewport(objects, selectedObjectIndex);

    if (m_showImGuiDemo)
        ImGui::ShowDemoWindow(&m_showImGuiDemo);
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
    ImGui::DockBuilderDockWindow("File Explorer", dockLeft);

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

    if (ImGui::BeginMenu("File"))
    {
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
        ImGui::MenuItem("File Explorer", nullptr, &m_showFileExplorer);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &m_showImGuiDemo);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Run"))
    {
        if (ImGui::MenuItem(m_playMode ? "Stop" : "Play", "F5"))
            m_playMode = !m_playMode;
        ImGui::EndMenu();
    }

    ImGui::TextDisabled("Project: %s", m_projectRoot.string().c_str());
    // Right-aligned viewport FPS.
    {
        const float fps = Utilities::GetViewportFPS();
        char buf[64]{};
        std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);

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

    if (ImGui::BeginTabBar("##ViewportTabs", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_Reorderable))
    {
        if (ImGui::BeginTabItem("Viewport"))
        {
			const ImVec2 availableSize = ImGui::GetContentRegionAvail();
			m_viewportSize = glm::vec2(availableSize.x, availableSize.y);

            // Viewport toolbar (edit gizmos + play mode)
            {
                const bool canEdit = !m_playMode;
                ImGui::TextDisabled("Gizmo:");
                ImGui::SameLine();

                ImGui::BeginDisabled(!canEdit);
                if (ImGui::SmallButton("Translate (W)")) m_gizmoMode = 0;
                ImGui::SameLine();
                if (ImGui::SmallButton("Rotate (E)")) m_gizmoMode = 1;
                ImGui::SameLine();
                if (ImGui::SmallButton("Scale (R)")) m_gizmoMode = 2;
                ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::SeparatorText(m_playMode ? "PLAYING" : "EDIT");
                ImGui::SameLine();
                if (ImGui::SmallButton(m_playMode ? "Stop (F5)" : "Play (F5)"))
                    m_playMode = !m_playMode;

                ImGui::Separator();
            }

            if (m_viewportTextureId != 0 && availableSize.x > 1.0f && availableSize.y > 1.0f)
            {
                ImGui::Image(
                    static_cast<ImTextureID>(m_viewportTextureId),
                    availableSize,
                    ImVec2(0.0f, 1.0f),
                    ImVec2(1.0f, 0.0f)
                );

                // Simple viewport gizmo: click+drag over the viewport image to transform the selected object.
                // - W/E/R switches mode (Translate/Rotate/Scale) when the viewport is hovered.
                // - This is intentionally a "data-first" gizmo until a full 3D handle renderer is added.
                const bool hasSelection = (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objects.size()));
                const bool imgHovered = ImGui::IsItemHovered();
                ImGuiIO& io = ImGui::GetIO();

                if (imgHovered)
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_W)) m_gizmoMode = 0;
                    if (ImGui::IsKeyPressed(ImGuiKey_E)) m_gizmoMode = 1;
                    if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoMode = 2;
                    if (ImGui::IsKeyPressed(ImGuiKey_F5)) m_playMode = !m_playMode;
                }

                if (!m_playMode && hasSelection)
                {
                    EditorObject& obj = objects[static_cast<size_t>(selectedObjectIndex)];

                    if (!m_gizmoDragging && imgHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    {
                        m_gizmoDragging = true;
                        m_gizmoDragStartMouse = glm::vec2(io.MousePos.x, io.MousePos.y);
                        m_gizmoStartPos = obj.position;
                        m_gizmoStartRotDeg = obj.rotationDeg;
                        m_gizmoStartScale = obj.scale;
                    }

                    if (m_gizmoDragging)
                    {
                        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                        {
                            m_gizmoDragging = false;
                        }
                        else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                        {
                            const glm::vec2 mouseNow(io.MousePos.x, io.MousePos.y);
                            const glm::vec2 delta = mouseNow - m_gizmoDragStartMouse;

                            // Tunables (screen-space -> world/deg scaling)
                            constexpr float kTranslate = 0.01f;
                            constexpr float kRotate = 0.15f;
                            constexpr float kScale = 0.01f;

                            if (m_gizmoMode == 0)
                            {
                                obj.position = m_gizmoStartPos + glm::vec3(delta.x * kTranslate, -delta.y * kTranslate, 0.0f);
                            }
                            else if (m_gizmoMode == 1)
                            {
                                obj.rotationDeg = m_gizmoStartRotDeg + glm::vec3(-delta.y * kRotate, delta.x * kRotate, 0.0f);
                            }
                            else if (m_gizmoMode == 2)
                            {
                                const float uniform = 1.0f + (-delta.y * kScale);
                                const float s = (uniform < 0.001f) ? 0.001f : uniform;
                                obj.scale = m_gizmoStartScale * s;
                            }
                        }
                    }

                    // Minimal on-viewport hint.
                    const char* modeName = (m_gizmoMode == 0) ? "Translate" : (m_gizmoMode == 1) ? "Rotate" : "Scale";
                    ImGui::SetCursorScreenPos(ImGui::GetItemRectMin() + ImVec2(10.0f, 10.0f));
                    ImGui::TextDisabled("%s%s", modeName, m_gizmoDragging ? " (dragging)" : "");
                }
            }

            else
            {
                ImGui::TextUnformatted("Viewport (waiting for scene render)");
                ImGui::Separator();
                ImGui::TextDisabled("Tip: the engine will render into a framebuffer and show it here.");
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Music"))
        {
			RenderMusicPlayer();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
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

    if (ImGui::Button("Create"))
    {
        objects.push_back(EditorObject{ "GameObject " + std::to_string(objects.size() + 1) });
        selectedObjectIndex = static_cast<int>(objects.size() - 1);
        Logging::Debug() << "Created GameObject.\n";
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete") && selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objects.size()))
    {
        Logging::Warning() << "Deleted GameObject '" << objects[selectedObjectIndex].name << "'.\n";
        objects.erase(objects.begin() + selectedObjectIndex);
        selectedObjectIndex = objects.empty() ? -1 : std::min(selectedObjectIndex, static_cast<int>(objects.size() - 1));
    }

    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(objects.size()); ++i)
    {
        EditorObject& obj = objects[i];
        ImGui::PushID(i);
        const bool selected = (selectedObjectIndex == i);
        const char* label = obj.name.c_str();
        if (ImGui::Selectable(label, selected))
            selectedObjectIndex = i;
        ImGui::PopID();
    }

    ImGui::End();
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

    EditorObject& obj = objects[selectedObjectIndex];

    ImGui::Checkbox("Enabled", &obj.enabled);
    ImGui::SameLine();
    ImGui::TextUnformatted(obj.name.c_str());

    ImGui::Separator();
    ImGui::TextUnformatted("Transform");
    ImGui::DragFloat3("Position", &obj.position.x, 0.05f);
    ImGui::DragFloat3("Rotation (deg)", &obj.rotationDeg.x, 0.25f);
    ImGui::DragFloat3("Scale", &obj.scale.x, 0.05f, 0.001f, 1000.0f);

    ImGui::Separator();
    ImGui::TextUnformatted("Material");
    ImGui::ColorEdit3("Tint", &obj.tint.x);

    ImGui::Separator();
    ImGui::TextUnformatted("Components");

    // Add Component
    {
        const char* preview = "Add Component...";
        if (ImGui::BeginCombo("##AddComponent", preview))
        {
            if (!obj.staticMesh && ImGui::Selectable("Static Mesh"))
                obj.staticMesh = StaticMeshEditorComponent{};
            if (!obj.skeletalMesh && ImGui::Selectable("Skeletal Mesh"))
                obj.skeletalMesh = SkeletalMeshEditorComponent{};
            if (!obj.camera && ImGui::Selectable("Camera"))
                obj.camera = CameraEditorComponent{};
            if (!obj.pointLight && ImGui::Selectable("Point Light"))
                obj.pointLight = PointLightEditorComponent{};

            ImGui::EndCombo();
        }
    }

    const auto setPathFromSelectedAsset = [this](std::string& outPath)
        {
            if (m_selectedAsset.empty() || !std::filesystem::exists(m_selectedAsset))
                return;

            std::error_code ec;
            const auto rel = std::filesystem::relative(m_selectedAsset, m_projectRoot, ec);
            outPath = ec ? m_selectedAsset.string() : rel.string();
        };

    if (obj.staticMesh)
    {
        if (ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputText("Mesh", &obj.staticMesh->meshPath);
            ImGui::SameLine();
            if (ImGui::SmallButton("Use Selected##StaticMesh"))
                setPathFromSelectedAsset(obj.staticMesh->meshPath);

            ImGui::InputText("Texture", &obj.staticMesh->texturePath);
            ImGui::SameLine();
            if (ImGui::SmallButton("Use Selected##StaticMeshTex"))
                setPathFromSelectedAsset(obj.staticMesh->texturePath);

            ImGui::TextDisabled("Note: static mesh importing/material binding is still being wired into the renderer.");

            if (ImGui::Button("Remove Static Mesh"))
                obj.staticMesh.reset();
        }
    }

    if (obj.skeletalMesh)
    {
        if (ImGui::CollapsingHeader("Skeletal Mesh", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputText("Mesh", &obj.skeletalMesh->meshPath);
            ImGui::SameLine();
            if (ImGui::SmallButton("Use Selected##SkelMesh"))
                setPathFromSelectedAsset(obj.skeletalMesh->meshPath);

            ImGui::InputText("Animation", &obj.skeletalMesh->animationPath);
            ImGui::SameLine();
            if (ImGui::SmallButton("Use Selected##SkelAnim"))
                setPathFromSelectedAsset(obj.skeletalMesh->animationPath);

            ImGui::DragInt("Anim Index", &obj.skeletalMesh->animationIndex, 1.0f, -1, 1024);

            ImGui::InputText("Texture", &obj.skeletalMesh->texturePath);
            ImGui::SameLine();
            if (ImGui::SmallButton("Use Selected##SkelTex"))
                setPathFromSelectedAsset(obj.skeletalMesh->texturePath);

            ImGui::TextDisabled("Note: skeletal animation/skinning is planned; this component is data-first for now.");

            if (ImGui::Button("Remove Skeletal Mesh"))
                obj.skeletalMesh.reset();
        }
    }

    if (obj.camera)
    {
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Primary", &obj.camera->primary);
            ImGui::SliderFloat("FOV (deg)", &obj.camera->fovDeg, 1.0f, 140.0f, "%.1f");
            ImGui::DragFloat("Near", &obj.camera->nearPlane, 0.01f, 0.001f, 100.0f, "%.3f");
            ImGui::DragFloat("Far", &obj.camera->farPlane, 1.0f, 1.0f, 50000.0f, "%.1f");

            ImGui::TextDisabled("Tip: press Play to possess the primary camera.");

            if (ImGui::Button("Remove Camera"))
                obj.camera.reset();
        }
    }

    if (obj.pointLight)
    {
        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::ColorEdit3("Color", &obj.pointLight->color.x);
            ImGui::DragFloat("Intensity", &obj.pointLight->intensity, 0.05f, 0.0f, 1000.0f, "%.2f");
            ImGui::DragFloat("Radius", &obj.pointLight->radius, 0.05f, 0.0f, 1000.0f, "%.2f");

            ImGui::TextDisabled("Note: currently visualized as a small proxy in the demo renderer.");

            if (ImGui::Button("Remove Point Light"))
                obj.pointLight.reset();
        }
    }

    ImGui::End();
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
        ImGui::TextDisabled("Select an object from the Hierarchy.");
        ImGui::End();
        return;
    }

    EditorObject& obj = objects[selectedObjectIndex];

    ImGui::TextDisabled("Selected: %s", obj.name.c_str());
    ImGui::Separator();

    // Simple preset list (tint-only "materials" for now).
    struct Preset { const char* name; glm::vec3 tint; };
    static constexpr Preset kPresets[] = {
        {"Default",   {1.0f, 1.0f, 1.0f}},
        {"Clay",      {0.76f, 0.68f, 0.62f}},
        {"Gold",      {1.0f, 0.78f, 0.25f}},
        {"Emerald",   {0.20f, 0.85f, 0.45f}},
        {"Magenta",   {0.95f, 0.25f, 0.85f}},
        {"Ice",       {0.45f, 0.80f, 1.00f}},
    };

    const char* preview = (obj.materialPreset >= 0 && obj.materialPreset < static_cast<int>(IM_ARRAYSIZE(kPresets)))
        ? kPresets[obj.materialPreset].name
        : "<custom>";

    if (ImGui::BeginCombo("Preset", preview))
    {
        for (int i = 0; i < static_cast<int>(IM_ARRAYSIZE(kPresets)); ++i)
        {
            const bool selected = (i == obj.materialPreset);
            if (ImGui::Selectable(kPresets[i].name, selected))
            {
                obj.materialPreset = i;
                obj.tint = kPresets[i].tint;
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::ColorEdit3("Tint", &obj.tint.x);
    ImGui::TextDisabled("Note: This is currently a tint-only demo material.");

    ImGui::End();
}

void EditorUI::RenderConsole()
{
    m_console.Render();

}

void EditorUI::DrawDirectoryBrowser(const char* id,
    const std::filesystem::path& root,
    std::filesystem::path& currentDir,
    std::filesystem::path* selectedPath)
{
    ImGui::PushID(id);

    // Breadcrumb
    {
        std::filesystem::path rel;
        std::error_code ec;
        rel = std::filesystem::relative(currentDir, root, ec);
        if (ec) rel.clear();

        if (ImGui::Button("Root"))
            currentDir = root;

        std::filesystem::path accum = root;
        if (!rel.empty())
        {
            for (const auto& part : rel)
            {
                ImGui::SameLine();
                ImGui::TextUnformatted("/");
                ImGui::SameLine();
                accum /= part;
                if (ImGui::SmallButton(part.string().c_str()))
                    currentDir = accum;
            }
        }
    }

    ImGui::Separator();

    // Directory list
    std::vector<std::filesystem::directory_entry> entries;
    {
        std::error_code ec;
        for (const auto& e : std::filesystem::directory_iterator(currentDir, ec))
        {
            if (ec) break;
            entries.push_back(e);
        }
    }

    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b)
        {
            const bool ad = a.is_directory();
            const bool bd = b.is_directory();
            if (ad != bd) return ad > bd;
            return a.path().filename().string() < b.path().filename().string();
        });

    if (ImGui::BeginChild("##dirList", ImVec2(0, 0), false))
    {
        if (currentDir != root)
        {
            if (ImGui::Selectable("..", false))
                currentDir = currentDir.parent_path();
        }

        for (const auto& e : entries)
        {
            const bool isDir = e.is_directory();
            const std::string name = e.path().filename().string();

            std::string label = isDir ? ("[DIR] " + name) : name;
            const bool isSelected = (selectedPath && *selectedPath == e.path());
            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                if (selectedPath) *selectedPath = e.path();
            }

            if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                currentDir = e.path();
            }
        }
        ImGui::EndChild();
    }

    ImGui::PopID();
}

void EditorUI::RenderContentBrowser()
{
    if (!ImGui::Begin("Content Browser"))
    {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Root: %s", m_resourcesRoot.string().c_str());
    DrawDirectoryBrowser("content", m_resourcesRoot, m_contentDir, &m_selectedAsset);

    ImGui::End();
}

void EditorUI::RenderFileExplorer()
{
    if (!ImGui::Begin("File Explorer"))
    {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Root: %s", m_projectRoot.string().c_str());
    DrawDirectoryBrowser("files", m_projectRoot, m_fileDir, &m_selectedFile);

    ImGui::End();
}