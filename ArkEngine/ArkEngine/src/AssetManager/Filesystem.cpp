#include "FileSystem.h"

#if defined _WIN32
#include <windows.h>
#elif defined (__APPLE__)
#include <mach-o/dyld.h>
#elif defined (__linux__)
#include <unistd.h>
#include <limits.h>
#endif

#include <algorithm>
#include <cctype>
#include <fstream>

namespace Engine
{
	namespace
	{
		std::string ToLower(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
				{
					return static_cast<char>(std::tolower(c));
				});
			return value;
		}

		std::filesystem::path ResolveCaseInsensitivePath(const std::filesystem::path& path)
		{
			if (std::filesystem::exists(path))
			{
				return path;
			}

			std::filesystem::path resolved = path.is_absolute() ? path.root_path() : std::filesystem::path();
			for (const auto& part : path.relative_path())
			{
				const auto candidate = resolved / part;
				if (std::filesystem::exists(candidate))
				{
					resolved = candidate;
					continue;
				}

				if (!std::filesystem::exists(resolved) || !std::filesystem::is_directory(resolved))
				{
					return path;
				}

				const auto target = ToLower(part.string());
				bool found = false;
				for (const auto& entry : std::filesystem::directory_iterator(resolved))
				{
					if (ToLower(entry.path().filename().string()) == target)
					{
						resolved = entry.path();
						found = true;
						break;
					}
				}

				if (!found)
				{
					return path;
				}
			}

			return resolved;
		}
	}

	std::filesystem::path FileSystem::GetExecutableFolder() const
	{
#if defined(_WIN32)
		char buf[MAX_PATH];
		GetModuleFileNameA(NULL, buf, MAX_PATH);
		return std::filesystem::path(buf).remove_filename();
#elif defined(__APPLE__)
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		std::string tmp(size, '\0');
		_NSGetExecutablePath(&tmp[0], &size);
		return std::filesystem::weakly_canonical(std::filesystem::path(tmp)).parent_path();
#elif defined (__linux__)
		return std::filesystem::weakly_canonical(std::filesystem::read_symlink("/proc/self/exe")).remove_filename();
#else
		return std::filesystem::current_path();
#endif

	}

	std::filesystem::path FileSystem::GetAssetsFolder() const
	{
#if defined (ASSETS_ROOT)
		auto path = std::filesystem::path(std::string(ASSETS_ROOT));
		if (std::filesystem::exists(path))
		{
			return path;
		}
#else

#endif
		return std::filesystem::weakly_canonical(GetExecutableFolder() / "Resources");

	}

	std::filesystem::path FileSystem::GetAssetFilePath(const std::string& relativePath) const
	{
		return ResolveCaseInsensitivePath(GetAssetsFolder() / std::filesystem::path(relativePath));
	}

	std::vector<char> FileSystem::LoadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			return {};
		}

		auto size = file.tellg();
		file.seekg(0);

		std::vector<char> buffer(size);

		if (!file.read(buffer.data(), size))
		{
			return {};
		}
		
		return buffer;
	}

	std::vector<char> FileSystem::LoadAssetFile(const std::string& relativePath)
	{
		return LoadFile(GetAssetFilePath(relativePath));
	}

	std::string FileSystem::LoadAssetFileText(const std::string& relativePath)
	{
		auto buffer = LoadAssetFile(relativePath);
		return std::string(buffer.begin(), buffer.end());
	}

	bool FileSystem::SaveAssetFileText(const std::string& relativePath, const std::string& contents)
	{
		auto path = GetAssetFilePath(relativePath);
		std::filesystem::create_directories(path.parent_path());

		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file.is_open())
		{
			return false;
		}

		file << contents;
		return file.good();
	}



}