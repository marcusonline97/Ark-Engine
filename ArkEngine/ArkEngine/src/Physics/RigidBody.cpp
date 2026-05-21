#include "RigidBody.h"

#include <Bullet3/btBulletCollisionCommon.h>
#include <Bullet3/btBulletDynamicsCommon.h>

namespace Engine
{
	RigidBody::RigidBody(BodyType type, const std::shared_ptr<Collider>& collider, float mass, float friction)
		: m_type(type), m_collider(collider), m_mass(mass), m_friction(friction)
	{

		if (!collider)
		{
			return;
		}

		btVector3 inertia(0, 0, 0);
		if (m_type == BodyType::Dynamic && mass > 0.0f && m_collider->GetShape())
		{

		}
	}

	RigidBody::~RigidBody()
	{

	}

	btRigidBody* RigidBody::GetBody()
	{

	}
	void RigidBody::SetAddedToWorld(bool added)
	{

	}

	bool RigidBody::IsAddedToWorld() const
	{

	}

	BodyType RigidBody::GetType() const
	{

	}

	void RigidBody::SetPosition(const glm::vec3& pos)
	{

	}

	glm::vec3 RigidBody::GetPosition() const
	{

	}

	void RigidBody::SetRotation(const glm::quat& rot)
	{

	}

	glm::quat RigidBody::GetRotation() const
	{

	}
}