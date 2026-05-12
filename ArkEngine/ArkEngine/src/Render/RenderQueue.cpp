#include "RenderQueue.h"
#include "Mesh.h"
#include "Render/Material.h"
#include "Graphics/GraphicsAPI.h"

namespace Engine
{
	void RenderQueue::Submit(const RenderCommand& command)
	{
		m_commands.push_back(command);
	}

	void RenderQueue::Draw(GraphicsAPI* graphicsAPI)
	{
		for (auto& command : m_commands)
		{
			graphicsAPI->BindMaterial(command.material);
			graphicsAPI->BindMesh(command.mesh);
			graphicsAPI->DrawMesh(command.mesh);
		}

		m_commands.clear();

	}
}