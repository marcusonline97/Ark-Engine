#include "Camera/CameraController.h"

#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Ark
{
	void CameraController::ProcessKeyboard(float dt, const CameraInput& input)
	{
		const float speedMul = input.fast ? 3.0f : 1.0f;
		const float move = moveSpeed * speedMul * dt;

		const glm::vec3 forward = GetForward();
		const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
		const glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));

		if (input.forward) position += forward * move;
		if (input.back)    position -= forward * move;
		if (input.left)    position -= right * move;
		if (input.right)   position += right * move;
		if (input.up)      position += worldUp * move;
		if (input.down)    position -= worldUp * move;
	}

	void CameraController::ProcessKeyboard(float dt, bool fwd, bool back, bool left, bool right, bool up, bool down)
	{
		CameraInput input{};
		input.forward = fwd;
		input.back = back;
		input.left = left;
		input.right = right;
		input.up = up;
		input.down = down;
		ProcessKeyboard(dt, input);
	}

	void CameraController::ProcessMouseDelta(float dx, float dy)
	{
		yawDeg += dx * lookSensitivity;
		pitchDeg -= dy * lookSensitivity;
		pitchDeg = std::clamp(pitchDeg, -89.0f, 89.0f);
	}

	glm::vec3 CameraController::GetForward() const
	{
		glm::vec3 front;
		const float pitch = glm::radians(pitchDeg);
		const float yaw = glm::radians(yawDeg);

		front.x = cos(yaw) * cos(pitch);
		front.y = sin(pitch);
		front.z = sin(yaw) * cos(pitch);

		return glm::normalize(front);
	}

	glm::mat4 CameraController::GetViewMatrix() const
	{
		const glm::vec3 forward = GetForward();
		return glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 CameraController::GetProjMatrix(float aspect, float fovDeg, float nearZ, float farZ) const
	{
		return glm::perspective(glm::radians(fovDeg), aspect, nearZ, farZ);
	}
}