#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// EnTT entity handle forward-declare (include full header where needed)
namespace entt { enum class entity : uint32_t; }

class CubeMesh;
class Material;

namespace Ark
{
	struct TagComponent
	{
		std::string Tag;
	};

	struct EnabledComponent
	{
		bool Enabled = true;
	};

	// Simple parent/child relationship for editor hierarchy + transform parenting.
	struct HierarchyComponent
	{
		entt::entity Parent{};
		std::vector<entt::entity> Children{};
	};


	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
		// Euler angles in degrees
		glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

		glm::mat4 ToMat4() const
		{
			const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1, 0, 0));

			const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0, 1, 0));

			const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0, 0, 1));

			const glm::mat4 rot = rotZ * rotY * rotX;

			return glm::translate(glm::mat4(1.0f), Translation) * rot * glm::scale(glm::mat4(1.0f), Scale);

		}

	};

	struct RenderComponent
	{
		CubeMesh* Mesh = nullptr;
		Material* MaterialPtr = nullptr;
		std::string ModelPath;
		std::string TexturePath;
	};

	// --- New world-placeable components (editor-facing) ---

	// 1) Static Meshes (fbx/obj) - transformable + material assignment
	struct StaticMeshComponent
	{
		// Relative path under Resources (preferred), or absolute path.
		std::string MeshPath;
		// Optional base color / albedo texture path.
		std::string BaseColorTexturePath;
		glm::vec3 Tint{ 1.0f, 1.0f, 1.0f };
	};

	// 2) Skeletal Meshes (fbx/obj with armature) - transformable + animations + material assignment
	struct SkeletalMeshComponent
	{
		std::string MeshPath;
		// Optional animation clip file (or embedded selection later).
		std::string AnimationPath;
		std::string BaseColorTexturePath;
		glm::vec3 Tint{ 1.0f, 1.0f, 1.0f };
	};

	// 3) Camera - possessable world camera
	struct CameraComponent
	{
		float FOV = 45.0f;
		float NearPlane = 0.1f;
		float FarPlane = 100.0f;
		bool bPossess = false;
	};

	// 4) Point Light - small sphere-based volumetric light (editor data)
	struct PointLightComponent
	{
		glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
		float Strength = 5.0f;
	};
}





