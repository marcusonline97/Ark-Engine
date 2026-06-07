#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
	class Mesh;

	class MeshManager
	{
	public:
		std::shared_ptr<Mesh> GetOrLoadMesh(const std::string& path);

	private:
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

	};
}