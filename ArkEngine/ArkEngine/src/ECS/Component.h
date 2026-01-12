#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CubeMesh;
class Material;

namespace Ark
{
	struct TagComponent
	{
		std::string Tag;
	};


	struct TransformComponent
	{
		glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
		// Euler angles in degrees
		glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

		glm::mat4 ToMat4() const
		{
			const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1, 0, 0));

			const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0, 1, 0));

			const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0, 0, 1));

			const glm::mat4 rot = rotZ * rotY * rotX;

			return glm::translate(glm::mat4(1.0f), Translation) * rot * glm::scale(glm::mat4(1.0f), Scale);

		}

	};

	struct RenderComponent
	{
		CubeMesh* Mesh = nullptr;
		Material* MaterialPtr = nullptr;
		std::string ModelPath;
		std::string TexturePath;
	};
}





