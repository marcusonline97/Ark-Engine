#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace Ark::Rendering
{
	class ObjGpuMesh;

	class MeshCache
	{
	public:
		const ObjGpuMesh* GetOrLoadObj(const std::string& absoluteObjPath);

	private:
		struct Entry
		{
			std::unique_ptr<ObjGpuMesh> mesh;
			bool failed = false;
		};

		std::unordered_map<std::string, Entry> m_cache;
	};
}