#include "PlayerControllerComponent.h"
#include "Input/InputManager.h"
#include "Core/ArkEngine.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>

namespace Engine
{
	void PlayerControllerComponent::Update(float deltaTime)
	{
		auto& inputManager = ArkEngine::GetInstance().GetInputManager();
		auto rotation = m_owner->GetRotation();

		if (inputManager.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			const auto& oldPos = inputManager.GetMousePositionOld();
			const auto& currentPos = inputManager.GetMousePositionCurrent();

			float deltaX = currentPos.x - oldPos.x;
			float deltaY = currentPos.y - oldPos.y;

			// rotation around the Y axis
			rotation.y -= deltaX * m_sensitivity * deltaTime;

			// rotation around the X axis
			rotation.x -= deltaY * m_sensitivity * deltaTime;

			m_owner->SetRotation(rotation);
		}

		glm::mat4 rotMat(1.0f);
		rotMat = glm::rotate(rotMat, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // X-Axis
		rotMat = glm::rotate(rotMat, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Y-Axis
		rotMat = glm::rotate(rotMat, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Z-Axis


		glm::vec3 front = glm::normalize(glm::vec3(rotMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
		glm::vec3 right = glm::normalize(glm::vec3(rotMat * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));

		auto position = m_owner->GetPosition();


		//Left/Right
		if(inputManager.IsKeyPressed(GLFW_KEY_A))
		{
			position -= right * m_moveSpeed * deltaTime;
		}

		else if (inputManager.IsKeyPressed(GLFW_KEY_D))
		{
			position += right * m_moveSpeed * deltaTime;
		}
		//Forward/Backward
		if (inputManager.IsKeyPressed(GLFW_KEY_W))
		{
			position += front * m_moveSpeed * deltaTime;
		}
		else if (inputManager.IsKeyPressed(GLFW_KEY_S))
		{
			position -= front * m_moveSpeed * deltaTime;
		}

		m_owner->SetPosition(position);
	}
}