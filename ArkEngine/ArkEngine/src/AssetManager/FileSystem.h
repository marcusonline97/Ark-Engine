#pragma once

#include <filesystem>
#include <string>
#include <vector>
namespace Engine
{
	class FileSystem
	{
	public:

		std::filesystem::path GetExecutableFolder() const;
		std::filesystem::path GetAssetsFolder() const;
		std::filesystem::path GetAssetFilePath(const std::string& relativePath) const;


		std::vector<char> LoadFile(const std::filesystem::path& path);
		std::vector<char> LoadAssetFile(const std::string& relativePath);
		std::string LoadAssetFileText(const std::string& relativePath);
		bool SaveAssetFileText(const std::string& relativePath, const std::string& contents);
	};
}