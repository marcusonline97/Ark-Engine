#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class BasicMesh;

namespace Ark::Rendering
{
	class MeshCache
	{
	public:
		const ::BasicMesh* GetOrLoadMesh(const std::string& absoluteMeshPath);
		const ::BasicMesh* GetOrLoadObj(const std::string& absoluteObjPath); // Old path , kept for backward compatibility. Will be removed in the future.

	private:
		struct Entry
		{
			std::unique_ptr<::BasicMesh> mesh;
			bool failed = false;
		};

		std::unordered_map<std::string, Entry> m_cache;
	};
}