#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Ark::Rendering
{
	enum class MtlTextureSemantic
	{
		Diffuse,     // map_Kd
		Specular,    // map_Ks
		Normal,      // map_Bump / bump
		Opacity,     // map_d
	};

	struct MtlMaterial
	{
		// Keyed by semantic; values are texture paths as written in the MTL.
		// Multiple entries are supported (some exporters write multiple map_* lines).
		std::unordered_map<MtlTextureSemantic, std::vector<std::string>> textures;
	};

	bool TryLoadMtlMaterial(const std::string& mtlPath, MtlMaterial& outMaterial);
}
