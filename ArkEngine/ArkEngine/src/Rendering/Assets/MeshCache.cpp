#include "MeshCache.h"


#include "Logger.h"
#include "ECS/Common/Basic_Mesh.h"

namespace Ark::Rendering
{
	const ::BasicMesh* MeshCache::GetOrLoadMesh(const std::string& absoluteMeshPath) 
	{
		if (absoluteMeshPath.empty())
			return nullptr;

		auto& entry = m_cache[absoluteMeshPath];
		if (entry.failed)
			return nullptr;

		if (!entry.mesh)
		{
			auto mesh = std::make_unique<::BasicMesh>();
			if (!mesh->LoadMesh(absoluteMeshPath))
			{
				entry.failed = true;
				Logging::Warning() << "Failed to load mesh: " << absoluteMeshPath << "\n";
				
				return nullptr;
			}
			entry.mesh = std::move(mesh);
		}
		return entry.mesh.get();
	}

	const ::BasicMesh* MeshCache::GetOrLoadObj(const std::string& absoluteObjPath)
	{
		return GetOrLoadMesh(absoluteObjPath);
	}
}