#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

namespace Ark
{
	struct CameraInput
	{
		bool forward = false;
		bool back = false;
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
		bool fast = false;
	};

	struct CameraController
	{
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		float pitchDeg = 0.0f;
		float yawDeg = -90.0f;
		float moveSpeed = 5.0f;
		float lookSensitivity = 0.1f;

		void ProcessKeyboard(float dt, const CameraInput& input);
		void ProcessKeyboard(float dt, bool fwd, bool back, bool left, bool right, bool up, bool down);
		void ProcessMouseDelta(float dx, float dy);

		glm::vec3 GetForward() const;
		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjMatrix(float aspect, float fovDeg, float nearZ, float farZ) const;
	};
}

