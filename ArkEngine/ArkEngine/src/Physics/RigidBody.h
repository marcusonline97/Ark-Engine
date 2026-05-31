#pragma once
#include "Collider.h"
#include "Physics/CollisionObject.h"

#include <glm/gtc/quaternion.hpp>

#include <memory>

class btRigidBody;

namespace Engine
{
	class GameObject;

	enum class BodyType
	{
		Static,
		Dynamic,
		Kinematic
	};

	class RigidBody : public CollisionObject
	{
	public:
		RigidBody(BodyType type, const std::shared_ptr<Collider>& collider, float mass, float friction);
		~RigidBody();
		btRigidBody* GetBody();
		void SetAddedToWorld(bool added);
		bool IsAddedToWorld() const;
		
		BodyType GetType() const;

		void SetPosition(const glm::vec3& pos);
		glm::vec3 GetPosition() const;

		void SetRotation(const glm::quat& rot);
		glm::quat GetRotation() const;

		void ApplyImpulse(const glm::vec3& impulse);
		void SetTrigger(bool trigger);
		bool IsTrigger() const;
		void SetOwner(GameObject* owner);
		GameObject* GetOwner() const;

	private:
		std::unique_ptr<btRigidBody> m_body;
		BodyType m_type = BodyType::Static;
		std::shared_ptr<Collider> m_collider;
		float m_mass = 0.0f;
		float m_friction = 0.5f;
		bool m_addedToWorld = false;
		bool m_isTrigger = false;
		GameObject* m_owner = nullptr;
	};
}