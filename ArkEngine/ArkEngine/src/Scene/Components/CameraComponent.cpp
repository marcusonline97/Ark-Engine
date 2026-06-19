#include "CameraComponent.h"
#include "Scene/GameObject.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
	void CameraComponent::Update(float deltaTime)
	{
		
	}

    glm::mat4 CameraComponent::GetViewMatrix() const
    {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::mat4_cast(m_owner->GetRotation());

        mat[3] = glm::vec4(m_owner->GetPosition(), 1.0f);

        if (m_owner->GetParent())
        {
            mat = m_owner->GetParent()->GetWorldTransform() * mat;
        }

        return glm::inverse(mat);
    }

	glm::mat4 CameraComponent::GetProjectionMatrix(float aspect) const
	{
		return glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
	}

	void CameraComponent::SetFOV(float fov)
	{
		m_fov = std::clamp(fov, 1.0f, 179.0f);
	}

	float CameraComponent::GetFOV() const
	{
		return m_fov;
	}

	void CameraComponent::SetNearPlane(float nearPlane)
	{
		m_nearPlane = nearPlane;
	}

	float CameraComponent::GetNearPlane() const
	{
		return m_nearPlane;
	}

	void CameraComponent::SetFarPlane(float farPlane)
	{
		m_farPlane = farPlane;
	}

	float CameraComponent::GetFarPlane() const
	{
		return m_farPlane;
	}

}