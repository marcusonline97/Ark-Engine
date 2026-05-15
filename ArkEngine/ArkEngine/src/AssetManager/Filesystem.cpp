#include "FileSystem.h"

#if defined _WIN32
#include <windows.h>
#elif defined (__APPLE__)
#include <mach-o/dyld.h>
#elif defined (__linux__)
#include <unistd.h>
#include <limits.h>
#endif

namespace Engine
{
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
}