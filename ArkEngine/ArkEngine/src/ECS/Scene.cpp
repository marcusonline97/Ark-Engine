#include "Scene.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ECS/Component.h"

namespace Ark
{
	entt::entity Scene::CreateEntity(const std::string& name)
	{
		const entt::entity e = m_registry.create();
		m_registry.emplace<TagComponent>(e, TagComponent{ name });
		m_registry.emplace<EnabledComponent>(e, EnabledComponent{ true });
		m_registry.emplace<HierarchyComponent>(e, HierarchyComponent{ entt::null, {} });
		m_registry.emplace<TransformComponent>(e, TransformComponent{});
		return e;
	}

	void Scene::DestroyEntity(entt::entity e)
	{
		if (e == entt::null || !m_registry.valid(e))
			return;

		// Unparent children (keep their world transforms as local by baking world->local at time of unparent)
		if (m_registry.any_of<HierarchyComponent>(e))
		{
			auto& h = m_registry.get<HierarchyComponent>(e);
			for (const entt::entity c : h.Children)
			{
				if (!m_registry.valid(c)) continue;
				SetParent(c, entt::null);
			}
		}

		// Remove from parent list
		SetParent(e, entt::null);

		m_registry.destroy(e);
	}

	HierarchyComponent& Scene::EnsureHierarchy(entt::entity e)
	{
		if (!m_registry.any_of<HierarchyComponent>(e))
			return m_registry.emplace<HierarchyComponent>(e, HierarchyComponent{ entt::null, {} });
		return m_registry.get<HierarchyComponent>(e);
	}

	entt::entity Scene::GetParent(entt::entity child) const
	{
		if (!m_registry.valid(child) || !m_registry.any_of<HierarchyComponent>(child))
			return entt::null;
		return m_registry.get<HierarchyComponent>(child).Parent;
	}

	const std::vector<entt::entity>& Scene::GetChildren(entt::entity parent) const
	{
		if (!m_registry.valid(parent) || !m_registry.any_of<HierarchyComponent>(parent))
			return m_emptyChildrenScratch;
		return m_registry.get<HierarchyComponent>(parent).Children;
	}

	glm::mat4 Scene::GetParentWorldTransform(entt::entity e) const
	{
		const entt::entity parent = GetParent(e);
		if (parent == entt::null)
			return glm::mat4(1.0f);
		return GetWorldTransform(parent);
	}

	glm::mat4 Scene::GetWorldTransform(entt::entity e) const
	{
		if (e == entt::null || !m_registry.valid(e) || !m_registry.any_of<TransformComponent>(e))
			return glm::mat4(1.0f);

		glm::mat4 world = m_registry.get<TransformComponent>(e).ToMat4();
		entt::entity p = GetParent(e);
		while (p != entt::null && m_registry.valid(p) && m_registry.any_of<TransformComponent>(p))
		{
			world = m_registry.get<TransformComponent>(p).ToMat4() * world;
			p = GetParent(p);
		}
		return world;
	}

	void Scene::SetParent(entt::entity child, entt::entity newParent)
	{
		if (child == entt::null || !m_registry.valid(child))
			return;
		if (newParent != entt::null && !m_registry.valid(newParent))
			newParent = entt::null;
		if (newParent == child)
			return;

		// Prevent cycles: cannot parent under own descendant
		{
			entt::entity p = newParent;
			while (p != entt::null)
			{
				if (p == child)
					return;
				p = GetParent(p);
			}
		}

		auto& childH = EnsureHierarchy(child);
		const entt::entity oldParent = childH.Parent;

		if (oldParent == newParent)
			return;

		// Keep world transform stable: compute new local = inverse(parentWorld) * oldWorld
		if (m_registry.any_of<TransformComponent>(child))
		{
			const glm::mat4 childWorld = GetWorldTransform(child);
			const glm::mat4 parentWorld = (newParent == entt::null) ? glm::mat4(1.0f) : GetWorldTransform(newParent);
			const glm::mat4 newLocal = glm::inverse(parentWorld) * childWorld;

			// Decompose matrix to Translation/Rotation/Scale (simple; assumes no shear)
			TransformComponent& tc = m_registry.get<TransformComponent>(child);
			tc.Translation = glm::vec3(newLocal[3]);

			// Extract scale from basis vectors
			const glm::vec3 basisX = glm::vec3(newLocal[0]);
			const glm::vec3 basisY = glm::vec3(newLocal[1]);
			const glm::vec3 basisZ = glm::vec3(newLocal[2]);
			tc.Scale = glm::vec3(glm::length(basisX), glm::length(basisY), glm::length(basisZ));

			glm::mat3 rotM(1.0f);
			if (tc.Scale.x != 0.0f) rotM[0] = basisX / tc.Scale.x;
			if (tc.Scale.y != 0.0f) rotM[1] = basisY / tc.Scale.y;
			if (tc.Scale.z != 0.0f) rotM[2] = basisZ / tc.Scale.z;

			// Convert to Euler degrees (XYZ)
			const float sy = -rotM[2][0];
			const float cy = std::sqrt(std::max(0.0f, 1.0f - sy * sy));
			float x = 0.0f, y = 0.0f, z = 0.0f;
			if (cy > 1e-5f)
			{
				x = std::atan2(rotM[2][1], rotM[2][2]);
				y = std::asin(sy);
				z = std::atan2(rotM[1][0], rotM[0][0]);
			}
			else
			{
				// Gimbal lock
				x = std::atan2(-rotM[1][2], rotM[1][1]);
				y = std::asin(sy);
				z = 0.0f;
			}
			tc.Rotation = glm::degrees(glm::vec3(x, y, z));
		}

		// Remove child from old parent children list
		if (oldParent != entt::null && m_registry.valid(oldParent) && m_registry.any_of<HierarchyComponent>(oldParent))
		{
			auto& oldH = m_registry.get<HierarchyComponent>(oldParent);
			oldH.Children.erase(std::remove(oldH.Children.begin(), oldH.Children.end(), child), oldH.Children.end());
		}

		// Set new parent
		childH.Parent = newParent;

		// Add to new parent's children list
		if (newParent != entt::null)
		{
			auto& newH = EnsureHierarchy(newParent);
			if (std::find(newH.Children.begin(), newH.Children.end(), child) == newH.Children.end())
				newH.Children.push_back(child);
		}
	}

	entt::entity Scene::GetPossessedCamera() const
	{
		auto view = m_registry.view<CameraComponent>();
		for (const entt::entity e : view)
		{
			const auto& c = view.get<CameraComponent>(e);
			if (c.bPossess)
				return e;
		}
		return entt::null;
	}

	void Scene::SetPossessedCamera(entt::entity cameraEntity)
	{
		// Untick all
		auto view = m_registry.view<CameraComponent>();
		for (const entt::entity e : view)
			view.get<CameraComponent>(e).bPossess = false;

		if (cameraEntity != entt::null && m_registry.valid(cameraEntity) && m_registry.any_of<CameraComponent>(cameraEntity))
			m_registry.get<CameraComponent>(cameraEntity).bPossess = true;
	}
}

