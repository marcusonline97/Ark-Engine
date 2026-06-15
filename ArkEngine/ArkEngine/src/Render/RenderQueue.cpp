#include "RenderQueue.h"
#include "Core/ArkEngine.h"
#include "Mesh.h"
#include "Render/Material.h"
#include "Graphics/GraphicsAPI.h"
#include "Graphics/ShaderProgram.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <utility>

namespace Engine
{
    void RenderQueue::Init()
    {
        m_mesh2D = Mesh::CreatePlane();
    }

    void RenderQueue::Submit(const RenderCommand& command)
    {
        m_commands.push_back(command);
    }

    void RenderQueue::Submit(const RenderCommand2D& command)
    {
        m_commands2D.push_back(command);
    }

    void RenderQueue::Submit(const RenderCommandUI& command)
    {
        m_commandsUI.push_back(command);
    }

    void RenderQueue::Draw(GraphicsAPI& graphicsAPI, const CameraData& cameraData, const std::vector<LightData>& lights)
    {
        // 3D
        auto defaultShader = graphicsAPI.GetDefaultShaderProgram();
        for (auto& command : m_commands)
        {
            graphicsAPI.BindMaterial(command.material);
			defaultShader->Bind();
			defaultShader->SetUniform("uModel", command.modelMatrix);
			defaultShader->SetUniform("uView", cameraData.viewMatrix);
			defaultShader->SetUniform("uProjection", cameraData.projectionMatrix);
			defaultShader->SetUniform("uCameraPos", cameraData.position);
			defaultShader->SetUniform("uSpecularStrength", ArkEngine::GetInstance().GetSpecularStrength());
			defaultShader->SetUniform("uHasSpecularMap", command.material->HasTextureParam("uSpecularMap") ? 1 : 0);
            
			const int lightCount = std::min(static_cast<int>(lights.size()), 8);
			defaultShader->SetUniform("uLightCount", lightCount);
            for (int li = 0; li < lightCount; ++li)
            {
                const std::string p = "uLights[" + std::to_string(li) + "].";
                defaultShader->SetUniform(p + "color", lights[li].color);
                defaultShader->SetUniform(p + "position", lights[li].position);
                defaultShader->SetUniform(p + "direction", lights[li].direction);
                defaultShader->SetUniform(p + "intensity", lights[li].intensity);
                defaultShader->SetUniform(p + "range", lights[li].range);
                defaultShader->SetUniform(p + "type", lights[li].type);
            }

            graphicsAPI.BindMesh(command.mesh);
            graphicsAPI.DrawMesh(command.mesh);
            graphicsAPI.UnbindMesh(command.mesh);
        }

        m_commands.clear();

        // 2D
        graphicsAPI.SetDepthTestEnabled(false);
        graphicsAPI.SetBlendMode(BlendMode::Alpha);
        const auto shaderProgram2D = graphicsAPI.GetDefault2DShaderProgram();
        shaderProgram2D->Bind();
        m_mesh2D->Bind();
        for (auto& command : m_commands2D)
        {
            // rendering
            shaderProgram2D->SetUniform("uModel", command.modelMatrix);
            shaderProgram2D->SetUniform("uView", cameraData.viewMatrix);
            shaderProgram2D->SetUniform("uProjection", cameraData.orthoMatrix);
            shaderProgram2D->SetUniform("uSize", command.size.x, command.size.y);
            shaderProgram2D->SetUniform("uPivot", command.pivot.x, command.pivot.y);
            shaderProgram2D->SetUniform("uUVMin", command.lowerLeftUV.x, command.lowerLeftUV.y);
            shaderProgram2D->SetUniform("uUVMax", command.upperRightUV.x, command.upperRightUV.y);
            shaderProgram2D->SetUniform("uColor", command.color);
            shaderProgram2D->SetTexture("uTex", command.texture);
            m_mesh2D->Draw();

        }
        m_mesh2D->Unbind();
        graphicsAPI.SetBlendMode(BlendMode::Disabled);
        graphicsAPI.SetDepthTestEnabled(true);
        m_commands2D.clear();

        // UI
        graphicsAPI.SetDepthTestEnabled(false);
        graphicsAPI.SetBlendMode(BlendMode::Alpha);
        for (auto& command : m_commandsUI)
        {
            glm::mat4 ortho = glm::ortho(
                0.0f, static_cast<float>(command.screenWidth),
                0.0f, static_cast<float>(command.screenHeight)
            );
            command.shaderProgram->Bind();
            command.shaderProgram->SetUniform("uProjection", ortho);

            command.mesh->Bind();

            uint32_t indexBase = 0;
            for (auto& batch : command.batches)
            {
                if (batch.texture)
                {
                    command.shaderProgram->SetUniform("uUseTexture", 1);
                    command.shaderProgram->SetTexture("uTex", batch.texture);
                }
                else
                {
                    command.shaderProgram->SetUniform("uUseTexture", 0);
                }
                command.mesh->DrawIndexedRange(indexBase, batch.indexCount);
                indexBase += batch.indexCount;
            }
            command.mesh->Unbind();
        }
        graphicsAPI.SetBlendMode(BlendMode::Disabled);
        graphicsAPI.SetDepthTestEnabled(true);
        m_commandsUI.clear();
    }

    std::vector<RenderCommand> RenderQueue::FlushCommands()
    {
        std::vector<RenderCommand> out = std::move(m_commands);
        m_commands.clear();
        return out;
    }
}