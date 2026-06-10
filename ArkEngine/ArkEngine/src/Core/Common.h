#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Engine
{
	class Texture;

	struct CameraData
	{
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 orthoMatrix;
		glm::vec3 position;
	};

	struct LightData
	{
		glm::vec3 color;
		glm::vec3 position;
		glm::vec3 direction;

		float intensity = 1.0f;
		float range = 10.0f;
		int type = 0;

	};

	struct UIBatch
	{
		Texture* texture = nullptr;
		uint32_t indexCount = 0;
	};
}