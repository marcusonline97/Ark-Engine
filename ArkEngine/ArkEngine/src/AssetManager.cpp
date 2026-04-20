#include "AssetManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <Logger.h>
#include <Rendering/Texture/Texture.h>

AssetManager& AssetManager::Instance()
{
    static AssetManager s_Instance;
    return s_Instance;
}

static std::filesystem::path SanitizeRelativePath(std::filesystem::path p)
{
    // Treat root-relative paths like "\ArkEngine\Resources\..." as relative ("ArkEngine/Resources/...").
    if (!p.empty())
    {
        const std::string s = p.generic_string();
        if (!s.empty() && (s[0] == '/' || s[0] == '\\'))
            return std::filesystem::path(s.substr(1));
    }

    return p;
}

std::filesystem::path AssetManager::ResolveAgainstLayouts(const std::filesystem::path& rel) const
{
    if (rel.empty())
        return rel;

    // If caller passed an absolute path, honor it.
    {
        std::error_code ec;
        const auto abs = std::filesystem::absolute(rel, ec).lexically_normal();
        if (!ec && abs.is_absolute() && std::filesystem::exists(abs))
            return abs;
    }

    const std::filesystem::path sanitizedRel = SanitizeRelativePath(rel);

    std::filesystem::path exeDir = std::filesystem::current_path();
#if defined(_WIN32)
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

    std::vector<std::filesystem::path> roots;
    roots.reserve(12);

    auto addRoot = [&](const std::filesystem::path& p)
        {
            const auto normalized = p.lexically_normal();
            if (normalized.empty())
                return;
            if (std::find(roots.begin(), roots.end(), normalized) == roots.end())
                roots.push_back(normalized);
        };

    addRoot(exeDir);
    addRoot(exeDir.parent_path());
    addRoot(exeDir.parent_path().parent_path());
    addRoot(exeDir.parent_path().parent_path().parent_path());

    std::error_code cwdEc;
    const std::filesystem::path cwd = std::filesystem::current_path(cwdEc);
    if (!cwdEc)
    {
        addRoot(cwd);
        addRoot(cwd.parent_path());
        addRoot(cwd.parent_path().parent_path());
    }

    std::vector<std::filesystem::path> candidates;
    candidates.reserve(roots.size() * 3);

    auto addCandidate = [&](const std::filesystem::path& p)
        {
            const auto normalized = p.lexically_normal();
            if (std::find(candidates.begin(), candidates.end(), normalized) == candidates.end())
                candidates.push_back(normalized);
        };

    for (const auto& root : roots)
    {
        addCandidate(root / sanitizedRel);
        addCandidate(root / "ArkEngine" / sanitizedRel);
        addCandidate(root / "ArkEngine" / "ArkEngine" / sanitizedRel);
    }

    for (const auto& candidate : candidates)
    {
        if (std::filesystem::exists(candidate))
            return candidate;
    }

    if (!candidates.empty())
    {
        std::cerr << "Asset not found at any of:\n";
        for (const auto& candidate : candidates)
            std::cerr << " - " << candidate.string() << "\n";
        std::cerr << "Working directory: " << std::filesystem::current_path().string() << "\n";
        return sanitizedRel; // fallback
    }

    return sanitizedRel;
}

std::string AssetManager::ResolveAssetPath(const std::string& relativePath) const
{
    return ResolveAgainstLayouts(relativePath).string();
}

Texture* AssetManager::LoadTexture2D(const std::string& relativePath, bool flipY, bool generateMipmaps)
{
    const std::lock_guard<std::mutex> lock(m_Mutex);

    const std::string resolved = ResolveAssetPath(relativePath);

    (void)generateMipmaps;

    const std::string cacheKey = resolved;

    auto it = m_TextureCache.find(cacheKey);
    if (it != m_TextureCache.end())
        return it->second.get();

    auto tex = std::make_unique<Texture>(GL_TEXTURE_2D, resolved);
    tex->SetFlipY(flipY);

    if (!tex->Load(true))
    {
        std::cerr << "Failed to load texture: " << resolved << "\n";
        return nullptr;
    }

    Texture* raw = tex.get();
    m_TextureCache.emplace(cacheKey, std::move(tex));
    return raw;
}

void AssetManager::Clear()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear();
}