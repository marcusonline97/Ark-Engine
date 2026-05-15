#pragma once

#include <filesystem>

namespace Engine
{
	class FileSystem
	{
	public:

		std::filesystem::path GetExecutableFolder() const;

		std::filesystem::path GetAssetsFolder() const;


	};
}