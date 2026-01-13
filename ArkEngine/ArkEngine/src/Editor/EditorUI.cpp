#include "EditorUI.h"

#include <algorithm>
#include <cctype>
#include <cstdio>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
static const char* LevelName(Logging::Level level)
{
    switch (level)
    {
    case Logging::Level::INIT: return "INIT";
    case Logging::Level::_ERROR: return "ERROR";
    case Logging::Level::WARNING: return "WARN";
    case Logging::Level::DEBUG: return "DEBUG";
    case Logging::Level::FATAL: return "FATAL";
    case Logging::Level::TODO: return "TODO";
    case Logging::Level::FUNCTION: return "FUNC";
    }
    return "LOG";
}

static ImVec4 LevelColor(Logging::Level level)
{
    switch (level)
    {
    case Logging::Level::_ERROR:   return ImVec4(0.95f, 0.25f, 0.25f, 1.0f);
    case Logging::Level::WARNING:  return ImVec4(0.95f, 0.75f, 0.20f, 1.0f);
    case Logging::Level::DEBUG:    return ImVec4(0.35f, 0.80f, 0.95f, 1.0f);
    case Logging::Level::FATAL:    return ImVec4(1.00f, 0.10f, 0.10f, 1.0f);
    case Logging::Level::TODO:     return ImVec4(0.35f, 0.95f, 0.35f, 1.0f);
    case Logging::Level::FUNCTION: return ImVec4(0.45f, 0.55f, 1.00f, 1.0f);
    case Logging::Level::INIT:     return ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
    }
    return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
}

static bool PassesLevelFilter(Logging::Level lvl,
    bool showDebug, bool showInfo, bool showWarn, bool showError, bool showFatal, bool showTodo, bool showFunc)
{
    switch (lvl)
    {
    case Logging::Level::DEBUG:    return showDebug;
    case Logging::Level::INIT:     return showInfo;
    case Logging::Level::WARNING:  return showWarn;
    case Logging::Level::_ERROR:   return showError;
    case Logging::Level::FATAL:    return showFatal;
    case Logging::Level::TODO:     return showTodo;
    case Logging::Level::FUNCTION: return showFunc;
    }
    return true;
}

static bool CaseInsensitiveFind(std::string_view haystack, std::string_view needle)
{
    if (needle.empty()) return true;
    if (haystack.size() < needle.size()) return false;

    auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };

    for (size_t i = 0; i + needle.size() <= haystack.size(); ++i)
    {
        bool ok = true;
        for (size_t j = 0; j < needle.size(); ++j)
        {
            if (lower(static_cast<unsigned char>(haystack[i + j])) != lower(static_cast<unsigned char>(needle[j])))
            {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
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

void EditorUI::Init()
{
    m_projectRoot = FindProjectRoot();
    m_resourcesRoot = FindResourcesRoot(m_projectRoot);
    m_contentDir = m_resourcesRoot;
    m_fileDir = m_projectRoot;

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
}


void EditorUI::PushLog(Logging::Level level, std::string_view msg)
{
    std::lock_guard<std::mutex> lock(m_consoleMutex);
    m_console.push_back(ConsoleEntry{ level, std::string(msg) });
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

    RenderViewport();

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

    ImGui::EndMenuBar();
}

void EditorUI::RenderViewport()
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
    if (!ImGui::Begin("Console"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        std::lock_guard<std::mutex> lock(m_consoleMutex);
        m_console.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_consoleAutoScroll);
    ImGui::SameLine();
    ImGui::Checkbox("Wrap", &m_consoleWrap);

    ImGui::Separator();
    ImGui::SetNextItemWidth(280.0f);
    ImGui::InputTextWithHint("##consoleFilter", "Filter text...", m_consoleFilter, sizeof(m_consoleFilter));

    ImGui::SameLine();
    ImGui::TextUnformatted("Levels:");
    ImGui::SameLine(); ImGui::Checkbox("INFO", &m_consoleShowInfo);
    ImGui::SameLine(); ImGui::Checkbox("DBG", &m_consoleShowDebug);
    ImGui::SameLine(); ImGui::Checkbox("WRN", &m_consoleShowWarning);
    ImGui::SameLine(); ImGui::Checkbox("ERR", &m_consoleShowError);
    ImGui::SameLine(); ImGui::Checkbox("FTL", &m_consoleShowFatal);
    ImGui::SameLine(); ImGui::Checkbox("TODO", &m_consoleShowTodo);
    ImGui::SameLine(); ImGui::Checkbox("FUNC", &m_consoleShowFunction);

    ImGui::Separator();

    ImGuiWindowFlags childFlags = ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::BeginChild("##consoleScroll", ImVec2(0, 0), false, childFlags);

    const std::string_view filter = m_consoleFilter;

    std::vector<ConsoleEntry> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_consoleMutex);
        snapshot = m_console;
    }

    for (const auto& e : snapshot)
    {
        if (!PassesLevelFilter(e.level,
            m_consoleShowDebug, m_consoleShowInfo, m_consoleShowWarning, m_consoleShowError, m_consoleShowFatal, m_consoleShowTodo, m_consoleShowFunction))
            continue;

        if (!CaseInsensitiveFind(e.message, filter))
            continue;

        ImGui::PushStyleColor(ImGuiCol_Text, LevelColor(e.level));
        ImGui::Text("[%s] ", LevelName(e.level));
        ImGui::PopStyleColor();
        ImGui::SameLine();

        if (m_consoleWrap)
            ImGui::TextWrapped("%s", e.message.c_str());
        else
            ImGui::TextUnformatted(e.message.c_str());
    }

    if (m_consoleAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f)
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
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