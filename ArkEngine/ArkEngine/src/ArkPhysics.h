#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace Ark::Physics
{
	// AABB

	struct AABB
	{
		glm::vec3 min{ 0.0f };
		glm::vec3 max{ 0.0f };

		// Contruct from the center and half extents
		static AABB FromCentre(const glm::vec3& centre, const glm::vec3& halfExtens)
		{
			return { centre - halfExtens, centre + halfExtens };
		}

		glm::vec3 Centre() const { return (min + max) * 0.5f; }
		glm::vec3 HalfExtents() const { return (max - min) * 0.5f; }

		// return the AABB translated to a new world position
		AABB Translated(const glm::vec3& worldPos) const
		{
			const glm::vec3 he = HalfExtents();
			return AABB::FromCentre(worldPos, he);
		}

		bool Overlaps(const AABB& other) const
		{
			return (min.x <= other.max.x && max.x >= other.min.x) &&
				(min.y <= other.max.y && max.y >= other.min.y) &&
				(min.z <= other.max.z && max.z >= other.min.z);
		}

		// Returns true and fills 'depth' with the minimum separation vector
		// needed to push 'this' out of 'other'.

		bool Penetration(const AABB& other, glm::vec3& outDepth) const
		{
			if (!Overlaps(other))
				return false;

			const glm::vec3 cDiff = Centre() - other.Centre();
			const glm::vec3 sumHE = HalfExtents() + other.HalfExtents();

			const glm::vec3 overlap = sumHE - glm::abs(cDiff);

			// Pick the axis with the smallest penetration depth
			if (overlap.x < overlap.y && overlap.x < overlap.z)
				outDepth = glm::vec3(glm::sign(cDiff.x) * overlap.x, 0.0f, 0.0f);
			else if (overlap.y < overlap.z)
				outDepth = glm::vec3(0.0f, glm::sign(cDiff.y) * overlap.y, 0.0f);
			else
				outDepth = glm::vec3(0.0f, 0.0f, glm::sign(cDiff.z) * overlap.z);

			return true;
		}
	};

	// Linetrace (raycast)

	struct LineTraceHit
	{
		bool hit = false;
		float distance = 0.0f; // distance from ray origin to impact point
		glm::vec3 point{ 0.0f }; // world-space impact point
		glm::vec3 normal{ 0.0f };  // surface normal at impact (axis-aligned)
		uint32_t objectId = 0;  // id of the EditorObject that was hit
	};


	// Slab method AABB vs ray. Returns false if no intersection or if the
   // intersection is behind the ray origin (tMin < 0).
	inline bool RayVsAABB(
		const glm::vec3& rayOrigin,
		const glm::vec3& rayDirNorm,
		const AABB& box,
		float maxDist,
		float outT)
	{
		const glm::vec3 invDir = 1.0f / rayDirNorm; // component-wise

		const glm::vec3 t0 = (box.min - rayOrigin) * invDir;
		const glm::vec3 t1 = (box.max - rayOrigin) * invDir;

		const glm::vec3 tMin3 = glm::min(t0, t1);
		const glm::vec3 tMax3 = glm::max(t0, t1);

		const float tMin = glm::max(glm::max(tMin3.x, tMin3.y), tMin3.z);
		const float tMax = glm::min(glm::min(tMax3.x, tMax3.y), tMax3.z);

		if(tMax < 0.0f || tMin > tMax  || tMin > maxDist)
			return false;

		outT = tMin >= 0.0f ? tMin : tMax;
		return true;
	}

	// Physics body (one per collideable EditorObject)

	enum class BodyType : uint8_t
	{
		Static = 0, // Immovable (terrain, walls, etc.)
		Dynamic = 1, // Affected by gravity and impulses
		Kinematic = 2 // Moved by code, not physics (moving platforms, player)
	};

	struct PhysicsBody
	{
		uint32_t objectId = 0; 
		BodyType type = BodyType::Static;

		// Local-space AABB (centred at origin). Translated to world-space
		// each frame using the owning object's position.
		AABB      localAABB{};

		// Runtime state (only meaningful for Dynamic/Kinematic bodies).
		glm::vec3 velocity{ 0.0f };
		bool      onGround = false;

		// Collision response flags
		bool      isTrigger = false; // Overlap events, no physical response
	};

		// -------------------------------------------------------------------------
		// Collision event
		// -------------------------------------------------------------------------

	struct CollisionEvent
	{
		uint32_t  bodyAId = 0;
		uint32_t  bodyBId = 0;
		glm::vec3 depth{ 0.0f }; // MTV: push A out of B by this vector
	};

	// Physics World

	class PhysicsWorld
	{
	public:
		static constexpr float kGravity = -9.81f;

		// ---------- body management ----------

		// Add or update a body. Call once per object when it enters the scene,
		// and again if its AABB or type changes.
		void AddOrUpdateBody(const PhysicsBody& body);
		void RemoveBody(uint32_t objectId);

		// Retrieve mutable body by object ID (returns nullptr if not found).
		PhysicsBody* GetBody(uint32_t objectId);
		const PhysicsBody* GetBody(uint32_t objectId) const;

		// ---------- per-frame ----------

		// Step the simulation by dt seconds.
		// Writes back corrected positions into 'outPositions' (indexed by objectId).
		// Only Dynamic/Kinematic bodies are moved.
		void Step(float dt,
			std::vector<std::pair<uint32_t, glm::vec3>>& outPositions,
			std::vector<CollisionEvent>& outEvents);

		// Apply an impulse to a dynamic body (e.g. jump).
		void ApplyImpulse(uint32_t objectId, const glm::vec3& impulse);

		// ---------- line trace ----------

		// Cast a ray against all registered body AABBs.
		// 'bodyPositions' maps objectId -> current world position.
		LineTraceHit LineTrace(
			const glm::vec3& origin,
			const glm::vec3& directionNorm,
			float            maxDistance,
			const std::vector<std::pair<uint32_t, glm::vec3>>& bodyPositions,
			uint32_t         ignoreObjectId = 0) const;

	private:
		std::vector<PhysicsBody> m_bodies;

		// Broadphase: returns pairs of potentially overlapping body indices.
		void BroadPhase(std::vector<std::pair<size_t, size_t>>& outPairs) const;
	};

}