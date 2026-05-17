#pragma once

#include "Core/Common.h"

#include <vector>
#include <glm/mat4x4.hpp>
namespace Engine
{
	class Mesh;
	class Material;
	class GraphicsAPI;

	struct RenderCommand
	{
		Mesh* mesh = nullptr;
		Material* material = nullptr;
		glm::mat4 modelMatrix;

	};

	

	class RenderQueue
	{
	public:
		void Submit(const RenderCommand& command);
		void Draw(GraphicsAPI& graphicsAPI, const CameraData& cameraData, const std::vector<LightData>& lights);
	private:
		std::vector<RenderCommand> m_commands;

	};
}