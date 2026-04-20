#include "AssetManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <system_error>

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

static bool PathExists(const std::filesystem::path& p)
{
    std::error_code ec;
    return std::filesystem::exists(p, ec) && !ec;
}

static std::filesystem::path QueryExecutableDirectory()
{
    std::filesystem::path exeDir = std::filesystem::current_path();

#if defined(_WIN32)
    char exePath[MAX_PATH] = { 0 };
    const DWORD len = GetModuleFileNameA(nullptr, exePath, static_cast<DWORD>(sizeof(exePath)));
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

    return exeDir.lexically_normal();
}

static void AddUniquePath(std::vector<std::filesystem::path>& paths, const std::filesystem::path& pathToAdd)
{
    const std::filesystem::path normalized = pathToAdd.lexically_normal();
    if (normalized.empty())
        return;

    if (std::find(paths.begin(), paths.end(), normalized) == paths.end())
        paths.push_back(normalized);
}

static void AddAncestorChain(std::vector<std::filesystem::path>& roots, const std::filesystem::path& start)
{
    if (start.empty())
        return;

    std::filesystem::path current = start.lexically_normal();
    for (;;)
    {
        AddUniquePath(roots, current);

        const std::filesystem::path parent = current.parent_path();
        if (parent.empty() || parent == current)
            break;
        current = parent;
    }
}

static void AddProjectSubroots(std::vector<std::filesystem::path>& roots)
{
    // Expand candidate roots by common nested project layouts.
    // Example: <solution>/ArkEngine/Resources relative to <solution>.
    const std::vector<std::filesystem::path> currentRoots = roots;
    for (const auto& root : currentRoots)
    {
        const auto ark1 = root / "ArkEngine";
        const auto ark2 = ark1 / "ArkEngine";

        if (PathExists(ark1) && std::filesystem::is_directory(ark1))
            AddUniquePath(roots, ark1);

        if (PathExists(ark2) && std::filesystem::is_directory(ark2))
            AddUniquePath(roots, ark2);
    }
}

std::filesystem::path AssetManager::ResolveAgainstLayouts(const std::filesystem::path& rel) const
{
    if (rel.empty())
        return rel;

    // If caller passed an absolute path, honor it.
    if (rel.is_absolute() && PathExists(rel))
        return rel.lexically_normal();

    const std::filesystem::path sanitizedRel = SanitizeRelativePath(rel);
    const std::filesystem::path exeDir = QueryExecutableDirectory();
    std::vector<std::filesystem::path> roots;
    roots.reserve(32);

    AddAncestorChain(roots, exeDir);

    std::error_code cwdEc;
    const std::filesystem::path cwd = std::filesystem::current_path(cwdEc);
    if (!cwdEc && !cwd.empty())
        AddAncestorChain(roots, cwd);

    AddProjectSubroots(roots);

    std::vector<std::filesystem::path> candidates;
    candidates.reserve(roots.size() + 2);

    auto addCandidate = [&](const std::filesystem::path& p)
        {
            AddUniquePath(candidates, p);
        };

    // Preserve an explicit relative-to-CWD check for IDE/debugger working directories.
    addCandidate(std::filesystem::path(".") / sanitizedRel);

    for (const auto& root : roots)
        addCandidate(root / sanitizedRel);

    for (const auto& candidate : candidates)
    {
        if (PathExists(candidate))
        {
            std::error_code ec;
            const auto absolute = std::filesystem::absolute(candidate, ec).lexically_normal();
            return ec ? candidate.lexically_normal() : absolute;
        }
    }

    if (!candidates.empty())
    {
        std::cerr << "Asset not found at any of:\n";

        for (const auto& candidate : candidates)
            std::cerr << "  " << candidate.string() << "\n";

        std::cerr << "Executable directory: " << exeDir.string() << "\n";
        std::cerr << "Working directory: " << std::filesystem::current_path().string() << "\n";
        return sanitizedRel; // fallback
    }

    return sanitizedRel;
}

std::string AssetManager::ResolveAssetPath(const std::string& relativePath) const
{
    if (relativePath.empty())
        return {};

    // Fast path: cache hit (lock only long enough to read the map).
    {
        const std::lock_guard<std::mutex> lock(m_Mutex);
        const auto it = m_PathCache.find(relativePath);
        if (it != m_PathCache.end())
            return it->second;
    }

    // Slow path: probe the filesystem without holding the lock.
    const std::string resolved = ResolveAgainstLayouts(relativePath).string();

    // Store result (re-acquire lock for the write).
    {
        const std::lock_guard<std::mutex> lock(m_Mutex);
        m_PathCache.emplace(relativePath, resolved);
    }

    return resolved;
}

Texture* AssetManager::LoadTexture2D(const std::string& relativePath, bool flipY, bool generateMipmaps)
{
    // Resolve the path outside the lock — ResolveAssetPath has its own locking
    // and may hit the filesystem on a cache miss. Holding m_Mutex across that
    // would block every other AssetManager call for the duration of disk I/O.
    const std::string resolved = ResolveAssetPath(relativePath);
    if (resolved.empty())
        return nullptr;

    (void)generateMipmaps;

    // Cache key includes flipY so the same image loaded both ways coexists.
    const std::string cacheKey = resolved + (flipY ? ":flip" : "");

    {
        const std::lock_guard<std::mutex> lock(m_Mutex);
        const auto it = m_TextureCache.find(cacheKey);
        if (it != m_TextureCache.end())
            return it->second.get();
    }

    // Load the texture outside the lock — this is the slow stb_image decode + GPU upload.
    auto tex = std::make_unique<Texture>(GL_TEXTURE_2D, resolved);
    tex->SetFlipY(flipY);

    if (!tex->Load(true))
    {
        std::cerr << "Failed to load texture: " << resolved << "\n";
        return nullptr;
    }

    // Insert under lock. Check again in case another thread loaded it concurrently.
    const std::lock_guard<std::mutex> lock(m_Mutex);
    const auto [it, inserted] = m_TextureCache.emplace(cacheKey, std::move(tex));
    return it->second.get();
}

void AssetManager::Clear()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear();
    m_PathCache.clear();
}

void AssetManager::ClearPathCache()
{
    const std::lock_guard<std::mutex> lock(m_Mutex);
    m_PathCache.clear();
}