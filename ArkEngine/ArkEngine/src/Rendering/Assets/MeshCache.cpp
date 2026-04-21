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
		
		// If we previously failed, and user is retrying, clear the failure flag
		// (allows retry after fixing file issues)
		if (entry.failed && entry.mesh == nullptr)
		{
			Logging::Debug() << "[MeshCache] Retrying previously failed mesh: " << absoluteMeshPath << "\n";
			entry.failed = false;
		}
		
		if (entry.failed)
			return nullptr;

		if (!entry.mesh)
		{
			auto mesh = std::make_unique<::BasicMesh>();
			Logging::Debug() << "[MeshCache] Loading new mesh: " << absoluteMeshPath << "\n";
			
			if (!mesh->LoadMesh(absoluteMeshPath))
			{
				entry.failed = true;
				Logging::Warning() << "Failed to load mesh: " << absoluteMeshPath << "\n";
				return nullptr;
			}
			
			Logging::Debug() << "[MeshCache] Successfully cached mesh: " << absoluteMeshPath << "\n";
			entry.mesh = std::move(mesh);
		}
		else
		{
			Logging::Debug() << "[MeshCache] Returning cached mesh: " << absoluteMeshPath << "\n";
		}
		
		return entry.mesh.get();
	}

	const ::BasicMesh* MeshCache::GetOrLoadObj(const std::string& absoluteObjPath)
	{
		return GetOrLoadMesh(absoluteObjPath);
	}
}