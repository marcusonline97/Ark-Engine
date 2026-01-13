#include "AssetManager.h"
#include <iostream>
#include <array>
#include <vector>

#include "Logger.h"
#include "Texture.h" // Ensure Texture declaration is visible or include its header if available.

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <unistd.h>
#endif

AssetManager& AssetManager::Instance()
{
    static AssetManager s_Instance;
    return s_Instance;
}

std::filesystem::path AssetManager::ResolveAgainstLayouts(const std::filesystem::path& rel) const
{
    // Executable directory
    std::filesystem::path exeDir;
#if defined(_WIN32)
    char exePath[MAX_PATH] = { 0 };
    DWORD len = GetModuleFileNameA(nullptr, exePath, static_cast<DWORD>(sizeof(exePath)));
    exeDir = std::filesystem::path(std::string(exePath, len)).parent_path();
#else
    std::array<char, 4096> exePath{};
    ssize_t len = ::readlink("/proc/self/exe", exePath.data(), exePath.size() - 1);
    if (len > 0)
    {
        exePath[static_cast<size_t>(len)] = '\0';
        exeDir = std::filesystem::path(exePath.data()).parent_path();
    }
    else
    {
        exeDir = std::filesystem::current_path();
    }
#endif

    // Try typical layouts.
    //
    // Note: depending on Visual Studio "Working Directory", callers may pass either:
    // - "Resources/..." (project-root relative)
    // - "ArkEngine/Resources/..." (solution-root relative)
    //
    // To be robust, we try both forms.
    std::vector<std::filesystem::path> relCandidates;
    relCandidates.push_back(rel);

    const std::string relNorm = rel.generic_string(); // uses '/'
    constexpr const char* kProjectPrefix = "ArkEngine/";
    constexpr const char* kResourcesPrefix = "Resources/";

    if (relNorm.rfind(kProjectPrefix, 0) == 0)
    {
        relCandidates.emplace_back(relNorm.substr(std::char_traits<char>::length(kProjectPrefix)));
    }
    else if (relNorm.rfind(kResourcesPrefix, 0) == 0)
    {
        relCandidates.emplace_back(std::string(kProjectPrefix) + relNorm);
    }

    std::vector<std::filesystem::path> tried;
    tried.reserve(relCandidates.size() * 3);

    for (const auto& relTry : relCandidates)
    {
        std::filesystem::path try1 = exeDir / relTry;                                                // next to exe
        std::filesystem::path try2 = exeDir.parent_path().parent_path() / relTry;                    // project root
        std::filesystem::path try3 = exeDir.parent_path().parent_path().parent_path() / relTry;      // solution root

        tried.push_back(try1);
        tried.push_back(try2);
        tried.push_back(try3);

        if (std::filesystem::exists(try1)) return try1;
        if (std::filesystem::exists(try2)) return try2;
        if (std::filesystem::exists(try3)) return try3;
    }

    std::cerr << "Asset not found at any of:\n";
    for (const auto& p : tried)
        std::cerr << " - " << p.string() << "\n";
    std::cerr << "Working directory: " << std::filesystem::current_path().string() << "\n";
    return rel; // fallback
}

std::string AssetManager::ResolveAssetPath(const std::string& relativePath) const
{
    return ResolveAgainstLayouts(relativePath).string();
}

Texture* AssetManager::LoadTexture2D(const std::string& relativePath, bool flipY)
{
    const std::lock_guard<std::mutex> lock(m_Mutex);

    const std::string resolved = ResolveAssetPath(relativePath);
    auto it = m_TextureCache.find(resolved);
    if (it != m_TextureCache.end())
        return it->second.get();

    // Create and cache
    auto tex = std::make_unique<Texture>();
    if(!tex->Load2D(resolved, true, true)) // srgb and mipmaps
    {
        std::cerr << "Failed to load texture: " << resolved << "\n";
        return nullptr;
	}
    Texture* raw = tex.get();
    m_TextureCache.emplace(resolved, std::move(tex));
    return raw;
}

void AssetManager::Clear()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear();
}