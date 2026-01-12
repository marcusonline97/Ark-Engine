#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

#include <glm/glm.hpp>

#include "Logger.h"

struct EditorObject
{
    std::string name = "GameObject";
    bool enabled = true;
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 rotationDeg{ 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
};

class EditorUI
{
public:
    void Init();
    void Shutdown();

    void Render(std::vector<EditorObject>& objects, int& selectedObjectIndex);

    // Thread-safe entry point for the Logging sink callback
    void PushLog(Logging::Level level, std::string_view msg);

private:
    struct ConsoleEntry {
        Logging::Level level{};
        std::string message;
    };

    void RenderDockspace();
    void RenderMenuBar();

    void RenderViewport();
    void RenderHierarchy(std::vector<EditorObject>& objects, int& selectedObjectIndex);
    void RenderInspector(std::vector<EditorObject>& objects, int& selectedObjectIndex);
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

    // Console
    std::mutex m_consoleMutex;
    std::vector<ConsoleEntry> m_console;
    bool m_consoleAutoScroll = true;
    bool m_consoleWrap = true;
    bool m_consoleShowDebug = true;
    bool m_consoleShowInfo = true;
    bool m_consoleShowWarning = true;
    bool m_consoleShowError = true;
    bool m_consoleShowFatal = true;
    bool m_consoleShowTodo = true;
    bool m_consoleShowFunction = true;
    char m_consoleFilter[256]{};
};