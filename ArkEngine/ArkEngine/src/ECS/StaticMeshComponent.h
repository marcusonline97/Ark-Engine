#pragma once

#include <string>
#include <utility>

namespace Ark
{
	enum class MeshPrimitive
	{
		None = 0,

		Cube,
		Quad,
		Sphere,
		Pyramid,
	};

	struct StaticMeshComponent
	{
		// If `Primitive != MeshPrimitive::None`, use the built-in primitive mesh.
		MeshPrimitive Primitive = MeshPrimitive::Cube;

		// If `Primitive == MeshPrimitive::None`, load from this asset path (.fbx/.obj).
		std::string MeshPath;

		// Optional bindings (paths are project-relative).
		std::string MaterialPath;
		std::string TexturePath;

		StaticMeshComponent() = default;

		static StaticMeshComponent MakePrimitive(MeshPrimitive primitive)
		{
			StaticMeshComponent c;
			c.Primitive = primitive;
			c.MeshPath.clear();
			return c;
		}

		static StaticMeshComponent MakeAsset(std::string meshPath)
		{
			StaticMeshComponent c;
			c.Primitive = MeshPrimitive::None;
			c.MeshPath = std::move(meshPath);
			return c;
		}

		bool UsesPrimitive() const noexcept { return Primitive != MeshPrimitive::None; }
		bool UsesAsset() const noexcept { return Primitive == MeshPrimitive::None && !MeshPath.empty(); }
	};
}