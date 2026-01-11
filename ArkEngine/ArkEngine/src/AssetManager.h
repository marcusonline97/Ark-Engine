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
    Texture* LoadTexture2D(const std::string& relativePath, bool flipY);

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

};