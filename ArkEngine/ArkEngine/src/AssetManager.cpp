#include "AssetManager.h"
#include <iostream>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Logger.h"
#include "Texture.h" 

AssetManager& AssetManager::Instance()
{
    static AssetManager s_Instance;
    return s_Instance;
}

std::filesystem::path AssetManager::ResolveAgainstLayouts(const std::filesystem::path& rel) const
{
    std::filesystem::path exeDir = std::filesystem::current_path();
#if defined(_WIN32)
    // Executable directoryw
    char exePath[MAX_PATH] = { 0 };
    DWORD len = GetModuleFileNameA(nullptr, exePath, static_cast<DWORD>(sizeof(exePath)));

    if (len > 0)
        exeDir = std::filesystem::path(std::string(exePath, len)).parent_path();
#else
    char exePath[4096] = { 0 };
    const ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0)
    {
        exePath[len] = '\0';
        exeDir = std::filesystem::path(std::string(exePath)).parent_path();
    }
#endif
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

    const std::string resolved = ResolveAssetPath(relativePath);
    auto it = m_TextureCache.find(resolved);
    if (it != m_TextureCache.end())
        return it->second.get();

    // Create and cache
    auto tex = std::make_unique<Texture>();
    if(!tex->Load2D(resolved, true, true, flipY)) // srgb and mipmaps
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