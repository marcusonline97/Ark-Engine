#pragma once

#include <atomic>
#include <string>
#include <unordered_map>

class Texture;

namespace Ark::Rendering
{
	class ResourceLoader;

	class TextureCache
	{
	public:
		explicit TextureCache(ResourceLoader* loader);

		TextureCache(const TextureCache&) = delete;
		TextureCache& operator=(const TextureCache&) = delete;

		Texture* GetOrLoad2D(const std::string& texturePath, bool flipY, bool generateMipmaps);

	private:
		struct Entry
		{
			std::atomic<Texture*> texture{ nullptr }; // owned by AssetManager cache

			bool requested = false;
			bool failed = false;
		};

		ResourceLoader* m_loader = nullptr;

		std::unordered_map<std::string, Entry> m_cacheWithMipmaps;
		std::unordered_map<std::string, Entry> m_cacheNoMipmaps;
	};
}