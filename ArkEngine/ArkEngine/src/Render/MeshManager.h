#pragma once

#include "Render/Mesh.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	class MeshManager
	{
	public:
		std::shared_ptr<Mesh> GetOrLoadMesh(const std::string& path);

	private:
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
	};
}
