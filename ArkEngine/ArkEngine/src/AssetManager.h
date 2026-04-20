#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <mutex>
#include <memory>

class Texture;

class AssetManager
{
public:
    static AssetManager& Instance();

    // Resolve a relative asset path against common project layouts.
    // Returns absolute path as string. If not found, returns the input unchanged.
    std::string ResolveAssetPath(const std::string& relativePath) const;

    // Texture cache (returns cached Texture*; ownership stays in manager)
    Texture* LoadTexture2D(const std::string& relativePath, bool flipY, bool generateMipmaps = true);

    // Optional: clear cache
    void Clear();

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::filesystem::path ResolveAgainstLayouts(const std::filesystem::path& rel) const;

private:
    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, std::unique_ptr<Texture>> m_TextureCache;

    // Resolved path cache: maps raw input string -> absolute resolved path string.
    // Avoids repeated filesystem probing for the same path every frame.
    // Invalidated by ClearPathCache() or Clear().
    mutable std::unordered_map<std::string, std::string> m_PathCache;

public:
    // Call this if assets move on disk at runtime (e.g. hot-reload).
    void ClearPathCache();

};