#include "TextureCache.h"

#include "AssetManager.h"
#include "AssetManager/ResourceLoader.h"
#include "Logger.h"

namespace Ark::Rendering
{
	TextureCache::TextureCache(ResourceLoader* loader)
		: m_loader(loader)
	{
	}

	Texture* TextureCache::GetOrLoad2D(const std::string& texturePath, bool flipY, bool generateMipmaps)
	{
		if (texturePath.empty())
			return nullptr;

		const std::string resolved = AssetManager::Instance().ResolveAssetPath(texturePath);

		auto& cache = generateMipmaps ? m_cacheWithMipmaps : m_cacheNoMipmaps;
		auto& entry = cache[resolved];

		if (entry.failed)
			return nullptr;

		if (Texture* already = entry.texture.load(std::memory_order_acquire))
			return already;

		if (entry.requested)
			return nullptr;

		entry.requested = true;

		if (!m_loader)
		{
			entry.texture.store(AssetManager::Instance().LoadTexture2D(texturePath, flipY, generateMipmaps), std::memory_order_release);
			if (!entry.texture.load(std::memory_order_acquire))
			{
				entry.failed = true;
				Logging::Warning() << "TextureCache: Failed to load texture: " << resolved << "\n";
			}
			return entry.texture.load(std::memory_order_acquire);
		}

		m_loader->Enqueue([resolved, texturePath, flipY, generateMipmaps, &entry]()
			{
				Texture* t = AssetManager::Instance().LoadTexture2D(texturePath, flipY, generateMipmaps);
				entry.texture.store(t, std::memory_order_release);
				if (!t)
				{
					entry.failed = true;
					Logging::Warning() << "TextureCache: Failed to load texture: " << resolved << "\n";
				}
			});

		return nullptr;
	}
}