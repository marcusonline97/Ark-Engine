#include "AssetManager.h"
#include <Windows.h>
#include <iostream>
#include "Texture.h" // Ensure Texture declaration is visible or include its header if available.

AssetManager& AssetManager::Instance()
{
    static AssetManager s_Instance;
    return s_Instance;
}

std::filesystem::path AssetManager::ResolveAgainstLayouts(const std::filesystem::path& rel) const
{
    // Executable directory
    char exePath[MAX_PATH] = { 0 };
    DWORD len = GetModuleFileNameA(nullptr, exePath, static_cast<DWORD>(sizeof(exePath)));
    std::filesystem::path exeDir = std::filesystem::path(std::string(exePath, len)).parent_path();

    // Try typical layouts
    std::filesystem::path try1 = exeDir / rel;                                                // next to exe
    std::filesystem::path try2 = exeDir.parent_path().parent_path() / rel;                    // project root
    std::filesystem::path try3 = exeDir.parent_path().parent_path().parent_path() / rel;      // solution root

    std::filesystem::path asset = try1;
    if (!std::filesystem::exists(asset)) asset = try2;
    if (!std::filesystem::exists(asset)) asset = try3;

    if (!std::filesystem::exists(asset))
    {
        std::cerr << "Asset not found at any of:\n";
        std::cerr << " - " << try1.string() << "\n";
        std::cerr << " - " << try2.string() << "\n";
        std::cerr << " - " << try3.string() << "\n";
        std::cerr << "Working directory: " << std::filesystem::current_path().string() << "\n";
        return rel; // fallback
    }
    return asset;
}

std::string AssetManager::ResolveAssetPath(const std::string& relativePath) const
{
    return ResolveAgainstLayouts(relativePath).string();
}

Texture* AssetManager::LoadTexture2D(const std::string& relativePath, bool flipY)
{
    const std::lock_guard<std::mutex> lock(m_Mutex);

    // Use resolved absolute path as the cache key to avoid duplicates
    const std::string resolved = ResolveAssetPath(relativePath);
    auto it = m_TextureCache.find(resolved);
    if (it != m_TextureCache.end())
        return it->second.get();

    // Create and cache
    auto tex = std::make_unique<Texture>(resolved, flipY);
    Texture* raw = tex.get();
    m_TextureCache.emplace(resolved, std::move(tex));
    return raw;
}

void AssetManager::Clear()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear();
}