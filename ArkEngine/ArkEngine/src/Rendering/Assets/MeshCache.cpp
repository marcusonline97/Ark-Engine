#include "MeshCache.h"

#include <filesystem>

#include "Logger.h"
#include "Rendering/Mesh/ObjGpuMesh.h"

namespace Ark::Rendering
{
	const ObjGpuMesh* MeshCache::GetOrLoadObj(const std::string& absoluteObjPath)
	{
		if (absoluteObjPath.empty())
			return nullptr;

		auto& entry = m_cache[absoluteObjPath];
		if (entry.failed)
			return nullptr;

		if (!entry.mesh)
		{
			auto mesh = std::make_unique<ObjGpuMesh>();
			if (!mesh->LoadFromObj(std::filesystem::path(absoluteObjPath)))
			{
				entry.failed = true;
				Logging::Warning() << "MeshCache: Failed to load OBJ: " << absoluteObjPath << "\n";
				return nullptr;
			}
			entry.mesh = std::move(mesh);
		}

		return entry.mesh.get();
	}
}