#include "Render/SkySphereRenderer.h"

#include "Core/ArkEngine.h"
#include "Graphics/GraphicsAPI.h"
#include "Graphics/ShaderProgram.h"
#include "Render/Mesh.h"

#include <GLAD/glad.h>
#include <glm/vec3.hpp>

namespace Engine
{
    namespace
    {
        const char* SkyVertexShader()
        {
            return R"(
            #version 330 core
            layout (location = 0) in vec3 position;

            out vec3 vDirection;

            uniform mat4 uView;
            uniform mat4 uProjection;

            void main()
            {
                vDirection = position;
                mat4 viewNoTranslation = mat4(mat3(uView));
                vec4 clipPos = uProjection * viewNoTranslation * vec4(position, 1.0);
                gl_Position = clipPos.xyww;
            }
            )";
        }

        const char* SkyFragmentShader()
        {
            return R"(
            #version 330 core

            in vec3 vDirection;
            out vec4 FragColor;

            uniform vec3 uTopColor;
            uniform vec3 uHorizonColor;
            uniform vec3 uBottomColor;
            uniform vec3 uSunDirection;

            void main()
            {
                vec3 dir = normalize(vDirection);
                float vertical = dir.y * 0.5 + 0.5;

                vec3 lower = mix(uBottomColor, uHorizonColor, smoothstep(0.0, 0.55, vertical));
                vec3 upper = mix(uHorizonColor, uTopColor, smoothstep(0.35, 1.0, vertical));
                vec3 sky = mix(lower, upper, smoothstep(0.35, 0.7, vertical));

                float sun = pow(max(dot(dir, normalize(uSunDirection)), 0.0), 512.0);
                float sunGlow = pow(max(dot(dir, normalize(uSunDirection)), 0.0), 16.0);
                sky += vec3(1.0, 0.82, 0.45) * sunGlow * 0.25;
                sky += vec3(1.0, 0.92, 0.72) * sun;

                FragColor = vec4(sky, 1.0);
            }
            )";
        }
    }

    bool SkySphereRenderer::Init()
    {
        if (m_sphere && m_shader)
        {
            return true;
        }

        auto& graphicsAPI = ArkEngine::GetInstance().GetGraphicsAPI();
        m_shader = graphicsAPI.CreateShaderProgram(SkyVertexShader(), SkyFragmentShader());
        m_sphere = Mesh::CreateSphere(1.0f, 48, 24);

        return m_sphere != nullptr && m_shader != nullptr;
    }

    void SkySphereRenderer::Shutdown()
    {
        m_sphere.reset();
        m_shader.reset();
    }

    void SkySphereRenderer::Render(const CameraData& cameraData)
    {
        if (!m_sphere || !m_shader)
        {
            return;
        }

        const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        GLboolean depthWriteMask = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteMask);

        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        m_shader->Bind();
        m_shader->SetUniform("uView", cameraData.viewMatrix);
        m_shader->SetUniform("uProjection", cameraData.projectionMatrix);
        m_shader->SetUniform("uTopColor", glm::vec3(0.05f, 0.18f, 0.46f));
        m_shader->SetUniform("uHorizonColor", glm::vec3(0.48f, 0.68f, 0.92f));
        m_shader->SetUniform("uBottomColor", glm::vec3(0.015f, 0.025f, 0.055f));
        m_shader->SetUniform("uSunDirection", glm::vec3(0.35f, 0.65f, 0.25f));

        m_sphere->Bind();
        m_sphere->Draw();
        m_sphere->Unbind();

        glDepthMask(depthWriteMask);
        if (depthWasEnabled) { glEnable(GL_DEPTH_TEST); }
        else { glDisable(GL_DEPTH_TEST); }

        if (blendWasEnabled) { glEnable(GL_BLEND); }
        else { glDisable(GL_BLEND); }
    }
}
