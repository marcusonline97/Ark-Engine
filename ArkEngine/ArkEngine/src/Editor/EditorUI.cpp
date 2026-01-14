#include "EditorUI.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <Utility/Utility.h>

#include <ECS/Scene.h>
#include <ECS/Component.h>
#include <Camera/Camera.h>

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

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

void EditorUI::Render(Ark::Scene& scene, entt::entity& selectedEntity)
{
    ImGuizmo::BeginFrame();

    RenderDockspace();

    if (m_showHierarchy)      RenderHierarchy(scene, selectedEntity);
    if (m_showInspector)      RenderInspector(scene, selectedEntity);
    if (m_showMaterials)      RenderMaterials(scene, selectedEntity);
    if (m_showConsole)        RenderConsole();
    if (m_showContentBrowser) RenderContentBrowser();
    if (m_showFileExplorer)   RenderFileExplorer();

    RenderViewport(scene, selectedEntity);

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

void EditorUI::RenderViewport(Ark::Scene& scene, entt::entity selectedEntity)
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

            if (m_viewportTextureId != 0 && availableSize.x > 1.0f && availableSize.y > 1.0f)
            {
                ImGui::Image(
                    static_cast<ImTextureID>(m_viewportTextureId),
                    availableSize,
                    ImVec2(0.0f, 1.0f),
                    ImVec2(1.0f, 0.0f)
                );

                // Overlay gizmo in the same rect as the viewport image.
                if (m_gizmoEnabled)
                {
                    const ImVec2 vpMin = ImGui::GetItemRectMin();
                    const ImVec2 vpSize = ImGui::GetItemRectSize();
                    RenderGizmo(scene, selectedEntity, vpMin, vpSize);
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

void EditorUI::RenderGizmo(Ark::Scene& scene, entt::entity selectedEntity, const ImVec2& viewportMin, const ImVec2& viewportSize)
{
    auto& reg = scene.Registry();
    if (selectedEntity == entt::null || !reg.valid(selectedEntity) || !reg.any_of<Ark::TransformComponent>(selectedEntity))
        return;

    // Hotkeys (only when the viewport is hovered)
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) m_gizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) m_gizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoOperation = ImGuizmo::SCALE;
        if (ImGui::IsKeyPressed(ImGuiKey_Q)) m_gizmoMode = (m_gizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
    }

    // Camera for gizmo: use possessed camera if available, otherwise a simple default
    glm::mat4 view(1.0f);
    glm::mat4 proj(1.0f);
    {
        entt::entity camE = scene.GetPossessedCamera();
        glm::vec3 camPos(0.0f, 0.0f, 3.0f);
        float pitch = 0.0f;
        float yaw = -90.0f;
        float fov = 45.0f;
        float nearP = 0.1f;
        float farP = 100.0f;

        if (camE != entt::null && reg.valid(camE) && reg.all_of<Ark::TransformComponent, Ark::CameraComponent>(camE))
        {
            const auto& ct = reg.get<Ark::TransformComponent>(camE);
            const auto& cc = reg.get<Ark::CameraComponent>(camE);
            camPos = ct.Translation;
            pitch = ct.Rotation.x;
            yaw = ct.Rotation.y;
            fov = cc.FOV;
            nearP = cc.NearPlane;
            farP = cc.FarPlane;
        }

        const float aspect = (viewportSize.y > 0.0f) ? (viewportSize.x / viewportSize.y) : 1.0f;
        ArkCamera cam(camPos, fov, aspect, nearP, farP);
        cam.SetRotation(pitch, yaw);
        view = cam.GetViewMatrix();
        proj = cam.GetProjectionMatrix();
    }

    // Gizmo setup
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(viewportMin.x, viewportMin.y, viewportSize.x, viewportSize.y);

    // Camera entities should not be scaled
    ImGuizmo::OPERATION op = static_cast<ImGuizmo::OPERATION>(m_gizmoOperation);
    if (reg.any_of<Ark::CameraComponent>(selectedEntity) && op == ImGuizmo::SCALE)
        op = ImGuizmo::TRANSLATE;

    glm::mat4 model = scene.GetWorldTransform(selectedEntity);
    const bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        op,
        static_cast<ImGuizmo::MODE>(m_gizmoMode),
        glm::value_ptr(model));

    if (changed && ImGuizmo::IsUsing())
        scene.SetWorldTransform(selectedEntity, model);
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

static const char* GetEntityName(Ark::Scene& scene, entt::entity e)
{
    auto& reg = scene.Registry();
    if (reg.valid(e) && reg.any_of<Ark::TagComponent>(e))
        return reg.get<Ark::TagComponent>(e).Tag.c_str();
    return "<Entity>";
}

static bool InputTextString(const char* label, std::string& value)
{
    // ImGui core InputText doesn't support std::string without imgui_stdlib;
    // keep a fixed buffer and copy on edit.
    char buf[512]{};
    if (!value.empty())
        std::snprintf(buf, sizeof(buf), "%s", value.c_str());
    if (ImGui::InputText(label, buf, sizeof(buf)))
    {
        value = buf;
        return true;
    }
    return false;
}

void EditorUI::RenderEntityTree(Ark::Scene& scene, entt::entity entity, entt::entity& selectedEntity)
{
    auto& reg = scene.Registry();
    if (!reg.valid(entity))
        return;

    const bool isSelected = (selectedEntity == entity);
    const auto& children = scene.GetChildren(entity);
    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        (children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) |
        (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

    const char* label = GetEntityName(scene, entity);
    const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(entity))), flags, "%s", label);

    if (ImGui::IsItemClicked())
        selectedEntity = entity;

    // Drag this entity
    if (ImGui::BeginDragDropSource())
    {
        const uint32_t payload = static_cast<uint32_t>(entity);
        ImGui::SetDragDropPayload("ARK_DND_ENTITY", &payload, sizeof(payload));
        ImGui::TextUnformatted(label);
        ImGui::EndDragDropSource();
    }

    // Accept reparent drops
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ARK_DND_ENTITY"))
        {
            if (payload->DataSize == sizeof(uint32_t))
            {
                const uint32_t droppedId = *static_cast<const uint32_t*>(payload->Data);
                const entt::entity dropped = static_cast<entt::entity>(droppedId);
                scene.SetParent(dropped, entity);
                selectedEntity = dropped;
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (opened)
    {
        for (const entt::entity c : children)
            RenderEntityTree(scene, c, selectedEntity);
        ImGui::TreePop();
    }
}

void EditorUI::RenderHierarchy(Ark::Scene& scene, entt::entity& selectedEntity)
{
    if (!ImGui::Begin("Hierarchy"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Create"))
        ImGui::OpenPopup("##CreateEntityPopup");
    ImGui::SameLine();
    if (ImGui::Button("Delete") && selectedEntity != entt::null && scene.Registry().valid(selectedEntity))
    {
        Logging::Warning() << "Deleted Entity '" << GetEntityName(scene, selectedEntity) << "'.\n";
        scene.DestroyEntity(selectedEntity);
        selectedEntity = entt::null;
    }

    if (ImGui::BeginPopup("##CreateEntityPopup"))
    {
        auto& reg = scene.Registry();

        if (ImGui::MenuItem("Empty GameObject"))
        {
            entt::entity e = scene.CreateEntity("GameObject");
            selectedEntity = e;
            Logging::Debug() << "Created GameObject.\n";
        }

        if (ImGui::MenuItem("Static Mesh"))
        {
            entt::entity e = scene.CreateEntity("StaticMesh");
            reg.emplace<Ark::StaticMeshComponent>(e, Ark::StaticMeshComponent{});
            selectedEntity = e;
            Logging::Debug() << "Created Static Mesh.\n";
        }

        if (ImGui::MenuItem("Skeletal Mesh"))
        {
            entt::entity e = scene.CreateEntity("SkeletalMesh");
            reg.emplace<Ark::SkeletalMeshComponent>(e, Ark::SkeletalMeshComponent{});
            selectedEntity = e;
            Logging::Debug() << "Created Skeletal Mesh.\n";
        }

        if (ImGui::MenuItem("Camera"))
        {
            entt::entity e = scene.CreateEntity("Camera");
            reg.emplace<Ark::CameraComponent>(e, Ark::CameraComponent{});

            // If this is the first camera, auto-possess it.
            if (scene.GetPossessedCamera() == entt::null)
                scene.SetPossessedCamera(e);

            selectedEntity = e;
            Logging::Debug() << "Created Camera.\n";
        }

        if (ImGui::MenuItem("Point Light"))
        {
            entt::entity e = scene.CreateEntity("PointLight");
            reg.emplace<Ark::PointLightComponent>(e, Ark::PointLightComponent{});
            selectedEntity = e;
            Logging::Debug() << "Created Point Light.\n";
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Render tree roots (entities with no parent)
    auto& reg = scene.Registry();
    reg.view<Ark::HierarchyComponent>().each([&](entt::entity e, const Ark::HierarchyComponent& h)
    {
        if (h.Parent == entt::null)
            RenderEntityTree(scene, e, selectedEntity);
    });

    // Drop on empty space to unparent (make root)
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ARK_DND_ENTITY"))
        {
            if (payload->DataSize == sizeof(uint32_t))
            {
                const uint32_t droppedId = *static_cast<const uint32_t*>(payload->Data);
                const entt::entity dropped = static_cast<entt::entity>(droppedId);
                scene.SetParent(dropped, entt::null);
                selectedEntity = dropped;
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

void EditorUI::RenderInspector(Ark::Scene& scene, entt::entity& selectedEntity)
{
    if (!ImGui::Begin("Inspector"))
    {
        ImGui::End();
        return;
    }

    auto& reg = scene.Registry();
    if (selectedEntity == entt::null || !reg.valid(selectedEntity))
    {
        ImGui::TextDisabled("Select an object from the Hierarchy.");
        ImGui::End();
        return;
    }

    // Header: Enabled + Name
    if (reg.any_of<Ark::EnabledComponent>(selectedEntity))
        ImGui::Checkbox("Enabled", &reg.get<Ark::EnabledComponent>(selectedEntity).Enabled);
    else
        ImGui::TextDisabled("Enabled: <missing>");

    ImGui::SameLine();
    if (reg.any_of<Ark::TagComponent>(selectedEntity))
        ImGui::TextUnformatted(reg.get<Ark::TagComponent>(selectedEntity).Tag.c_str());
    else
        ImGui::TextUnformatted("<Entity>");

    ImGui::Separator();
    if (reg.any_of<Ark::TransformComponent>(selectedEntity))
    {
        auto& t = reg.get<Ark::TransformComponent>(selectedEntity);
        ImGui::TextUnformatted("Transform");
        ImGui::DragFloat3("Position", &t.Translation.x, 0.05f);
        ImGui::DragFloat3("Rotation (deg)", &t.Rotation.x, 0.25f);

        const bool isCamera = reg.any_of<Ark::CameraComponent>(selectedEntity);
        if (!isCamera)
            ImGui::DragFloat3("Scale", &t.Scale.x, 0.05f, 0.001f, 1000.0f);
        else
            ImGui::TextDisabled("Scale: (disabled for Camera)");
    }
    else
    {
        ImGui::TextDisabled("Transform: <missing>");
    }

    ImGui::Separator();
    // Component-specific sections
    if (reg.any_of<Ark::StaticMeshComponent>(selectedEntity))
    {
        auto& sm = reg.get<Ark::StaticMeshComponent>(selectedEntity);
        ImGui::TextUnformatted("Static Mesh");
        InputTextString("Mesh Path", sm.MeshPath);
        InputTextString("BaseColor Texture", sm.BaseColorTexturePath);
        ImGui::ColorEdit3("Tint", &sm.Tint.x);
    }

    if (reg.any_of<Ark::SkeletalMeshComponent>(selectedEntity))
    {
        auto& sk = reg.get<Ark::SkeletalMeshComponent>(selectedEntity);
        ImGui::TextUnformatted("Skeletal Mesh");
        InputTextString("Mesh Path", sk.MeshPath);
        InputTextString("Animation Path", sk.AnimationPath);
        InputTextString("BaseColor Texture", sk.BaseColorTexturePath);
        ImGui::ColorEdit3("Tint", &sk.Tint.x);
        ImGui::TextDisabled("Animation playback is pending a runtime animator.");
    }

    if (reg.any_of<Ark::CameraComponent>(selectedEntity))
    {
        auto& cam = reg.get<Ark::CameraComponent>(selectedEntity);
        ImGui::TextUnformatted("Camera");
        ImGui::SliderFloat("FOV", &cam.FOV, 10.0f, 120.0f, "%.1f deg");
        ImGui::DragFloat("Near", &cam.NearPlane, 0.01f, 0.001f, 10.0f);
        ImGui::DragFloat("Far", &cam.FarPlane, 1.0f, 10.0f, 10000.0f);

        bool possess = cam.bPossess;
        if (ImGui::Checkbox("bPossess", &possess))
        {
            if (possess)
                scene.SetPossessedCamera(selectedEntity);
            else
                cam.bPossess = false;
        }
    }

    if (reg.any_of<Ark::PointLightComponent>(selectedEntity))
    {
        auto& pl = reg.get<Ark::PointLightComponent>(selectedEntity);
        ImGui::TextUnformatted("Point Light");
        ImGui::ColorEdit3("Color", &pl.Color.x);
        ImGui::DragFloat("Strength", &pl.Strength, 0.1f, 0.0f, 100000.0f);
    }

    ImGui::End();
}
void EditorUI::RenderMaterials(Ark::Scene& scene, entt::entity& selectedEntity)
{
    if (!ImGui::Begin("Materials"))
    {
        ImGui::End();
        return;
    }

    auto& reg = scene.Registry();
    if (selectedEntity == entt::null || !reg.valid(selectedEntity))
    {
        ImGui::TextDisabled("Select an object from the Hierarchy.");
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("Selected: %s", GetEntityName(scene, selectedEntity));
    ImGui::Separator();

    // Tint-only presets (applies to StaticMeshComponent / SkeletalMeshComponent)
    struct Preset { const char* name; glm::vec3 tint; };
    static constexpr Preset kPresets[] = {
        {"Default",   {1.0f, 1.0f, 1.0f}},
        {"Clay",      {0.76f, 0.68f, 0.62f}},
        {"Gold",      {1.0f, 0.78f, 0.25f}},
        {"Emerald",   {0.20f, 0.85f, 0.45f}},
        {"Magenta",   {0.95f, 0.25f, 0.85f}},
        {"Ice",       {0.45f, 0.80f, 1.00f}},
    };

    auto applyTintPreset = [&](const glm::vec3& tint)
    {
        if (reg.any_of<Ark::StaticMeshComponent>(selectedEntity))
            reg.get<Ark::StaticMeshComponent>(selectedEntity).Tint = tint;
        if (reg.any_of<Ark::SkeletalMeshComponent>(selectedEntity))
            reg.get<Ark::SkeletalMeshComponent>(selectedEntity).Tint = tint;
    };

    if (ImGui::BeginCombo("Preset", "Select..."))
    {
        for (const auto& p : kPresets)
        {
            if (ImGui::Selectable(p.name))
                applyTintPreset(p.tint);
        }
        ImGui::EndCombo();
    }

    if (reg.any_of<Ark::StaticMeshComponent>(selectedEntity))
        ImGui::ColorEdit3("Tint (StaticMesh)", &reg.get<Ark::StaticMeshComponent>(selectedEntity).Tint.x);
    if (reg.any_of<Ark::SkeletalMeshComponent>(selectedEntity))
        ImGui::ColorEdit3("Tint (SkeletalMesh)", &reg.get<Ark::SkeletalMeshComponent>(selectedEntity).Tint.x);

    ImGui::TextDisabled("Note: Full material system wiring is in progress (texture slots next).");

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