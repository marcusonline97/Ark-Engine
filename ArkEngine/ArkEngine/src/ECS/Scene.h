#pragma once

#include <string>
#include <vector>

#include <entt.hpp>
#include <glm/mat4x4.hpp>

namespace Ark
{
	struct CameraComponent;
	struct HierarchyComponent;
	struct TransformComponent;

	// Minimal ECS scene wrapper for the editor/runtime.
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		entt::registry& Registry() { return m_registry; }
		const entt::registry& Registry() const { return m_registry; }

		entt::entity CreateEntity(const std::string& name);
		void DestroyEntity(entt::entity e);

		// Parenting
		void SetParent(entt::entity child, entt::entity newParent);
		entt::entity GetParent(entt::entity child) const;
		const std::vector<entt::entity>& GetChildren(entt::entity parent) const;

		// Camera possession
		entt::entity GetPossessedCamera() const;
		void SetPossessedCamera(entt::entity cameraEntity);

		// Transform helpers (local TransformComponent is stored; world is derived by parenting)
		glm::mat4 GetWorldTransform(entt::entity e) const;
		glm::mat4 GetParentWorldTransform(entt::entity e) const;

	private:
		HierarchyComponent& EnsureHierarchy(entt::entity e);

	private:
		entt::registry m_registry;
		mutable std::vector<entt::entity> m_emptyChildrenScratch{};
	};
}

