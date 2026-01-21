#include "MtlMaterial.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Logger.h"

namespace Ark::Rendering
{
	static inline void LTrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
	}

	static inline void RTrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
	}

	static inline void Trim(std::string& s)
	{
		LTrim(s);
		RTrim(s);
	}

	static inline bool StartsWith(const std::string& s, const char* prefix)
	{
		const size_t n = std::char_traits<char>::length(prefix);
		return s.size() >= n && s.compare(0, n, prefix) == 0;
	}

	// Very small subset: consumes `map_*` lines and returns the *last token* as the path.
	// This handles common options like: map_Kd -s 1 1 1 textures/diffuse.png
	static bool TryParseMapLine(const std::string& line, std::string& outPath)
	{
		std::istringstream iss(line);
		std::string token;

		// discard directive token (map_Kd, etc.)
		if (!(iss >> token))
			return false;

		// Collect tokens; keep last token as best-effort.
		std::string last;
		while (iss >> token)
			last = token;

		if (last.empty())
			return false;

		outPath = last;
		return true;
	}

	bool TryLoadMtlMaterial(const std::string& mtlPath, MtlMaterial& outMaterial)
	{
		outMaterial = {};

		if (mtlPath.empty())
			return false;

		std::ifstream in(mtlPath);
		if (!in.is_open())
		{
			Logging::Warning() << "TryLoadMtlMaterial: failed to open '" << mtlPath << "'\n";
			return false;
		}

		std::string line;
		while (std::getline(in, line))
		{
			Trim(line);
			if (line.empty() || StartsWith(line, "#"))
				continue;

			std::string texPath;

			if (StartsWith(line, "map_Kd"))
			{
				if (TryParseMapLine(line, texPath))
					outMaterial.textures[MtlTextureSemantic::Diffuse].push_back(texPath);
			}
			else if (StartsWith(line, "map_Ks"))
			{
				if (TryParseMapLine(line, texPath))
					outMaterial.textures[MtlTextureSemantic::Specular].push_back(texPath);
			}
			else if (StartsWith(line, "map_Bump") || StartsWith(line, "bump"))
			{
				if (TryParseMapLine(line, texPath))
					outMaterial.textures[MtlTextureSemantic::Normal].push_back(texPath);
			}
			else if (StartsWith(line, "map_d"))
			{
				if (TryParseMapLine(line, texPath))
					outMaterial.textures[MtlTextureSemantic::Opacity].push_back(texPath);
			}
		}

		return !outMaterial.textures.empty();
	}
}