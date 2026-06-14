#include "Render/DeferredRenderer.h"

#include "Core/ArkEngine.h"
#include "Graphics/GraphicsAPI.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Texture.h"
#include "Logger.h"
#include "Render/Material.h"
#include "Render/Mesh.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    namespace
    {
        constexpr int kMaxLights = 16;

        const char* GeometryVertexShader()
        {
            return R"(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec3 color;
            layout (location = 2) in vec2 uv;
            layout (location = 3) in vec3 normal;

            out vec3 vFragPos;
            out vec3 vNormal;
            out vec2 vUV;

            uniform mat4 uModel;
            uniform mat4 uView;
            uniform mat4 uProjection;

            void main()
            {
                vec4 worldPos = uModel * vec4(position, 1.0);
                vFragPos = worldPos.xyz;
                vNormal = normalize(transpose(inverse(mat3(uModel))) * normal);
                vUV = uv;
                gl_Position = uProjection * uView * worldPos;
            }
            )";
        }

        const char* GeometryFragmentShader()
        {
            return R"(
            #version 330 core

            layout (location = 0) out vec4 gPosition;
            layout (location = 1) out vec4 gNormal;
            layout (location = 2) out vec4 gAlbedoSpec;

            in vec3 vFragPos;
            in vec3 vNormal;
            in vec2 vUV;

            uniform sampler2D baseColorTexture;
            uniform sampler2D specularMap;
            uniform int       uHasSpecularMap;
            uniform float     uSpecularStrength;

            void main()
            {
                vec3 albedo = texture(baseColorTexture, vUV).rgb;
                gPosition = vec4(vFragPos, 1.0);
                gNormal = vec4(normalize(vNormal), 1.0);

                float specStrength = uSpecularStrength;
                if (uHasSpecularMap != 0)
                {
                    specStrength *= texture(specularMap, vUV).r;
                }

                gAlbedoSpec = vec4(albedo, max(specStrength, 0.0));
            }
            )";
        }

        const char* ShadowVertexShader()
        {
            return R"(
            #version 330 core
            layout (location = 0) in vec3 position;

            uniform mat4 uLightSpaceMatrix;
            uniform mat4 uModel;

            void main()
            {
                gl_Position = uLightSpaceMatrix * uModel * vec4(position, 1.0);
            }
            )";
        }

        const char* ShadowFragmentShader()
        {
            return R"(
            #version 330 core

            void main()
            {
            }
            )";
        }

        const char* LightingVertexShader()
        {
            return R"(
            #version 330 core
            layout (location = 0) in vec2 position;

            out vec2 vUV;

            void main()
            {
                vUV = position;
                gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
            }
            )";
        }

        const char* LightingFragmentShader()
        {
            return R"(
            #version 330 core

            struct Light
            {
                vec3 position;
                vec3 color;
                vec3 direction;
                float intensity;
                float range;
                int type;
            };

            const int MAX_LIGHTS = 16;

            in vec2 vUV;
            out vec4 FragColor;

            uniform sampler2D uGPosition;
            uniform sampler2D uGNormal;
            uniform sampler2D uGAlbedoSpec;
            uniform sampler2D uShadowMap;

            uniform Light uLights[MAX_LIGHTS];
            uniform int uLightCount;
            uniform int uShadowLightIndex;
            uniform int uHasShadow;
            uniform float uShadowStrength;
            uniform vec3 uCameraPos;
            uniform mat4 uLightSpaceMatrix;

            float ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightDir)
            {
                if (uHasShadow == 0)
                    return 0.0;

                vec4 fragPosLightSpace = uLightSpaceMatrix * vec4(fragPos, 1.0);
                vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                projCoords = projCoords * 0.5 + 0.5;

                if (projCoords.z > 1.0)
                    return 0.0;

                float currentDepth = projCoords.z;
                float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
                vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
                float shadow = 0.0;

                for (int x = -1; x <= 1; ++x)
                {
                    for (int y = -1; y <= 1; ++y)
                    {
                        float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                    }
                }

                return shadow / 9.0;
            }

            void main()
            {
                vec3 fragPos = texture(uGPosition, vUV).rgb;
                vec3 normalSample = texture(uGNormal, vUV).rgb;
                if (length(normalSample) < 0.0001)
                {
                    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                    return;
                }

                vec3 normal = normalize(normalSample);
                vec4 albedoSpec = texture(uGAlbedoSpec, vUV);
                vec3 albedo = albedoSpec.rgb;
                float specularStrength = albedoSpec.a;

                vec3 viewDir = normalize(uCameraPos - fragPos);
                vec3 lighting = albedo * 0.15;

                for (int i = 0; i < uLightCount; ++i)
                {
                    vec3 lightDir;
                    vec3 radiance = uLights[i].color * uLights[i].intensity;
                    float attenuation = 1.0;

                    if (uLights[i].type == 1)
                    {
                        vec3 toLight = uLights[i].position - fragPos;
                        float distanceToLight = length(toLight);
                        if (distanceToLight > uLights[i].range)
                            continue;

                        lightDir = toLight;
                        attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);
                    }
                    else
                    {
                        lightDir = -uLights[i].direction;
                    }

                    float lightDirLength = length(lightDir);
                    if (lightDirLength < 0.0001)
                        continue;
                    lightDir /= lightDirLength;

                    float diff = max(dot(normal, lightDir), 0.0);
                    vec3 halfwayDir = normalize(lightDir + viewDir);
                    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

                    float shadow = 0.0;
                    if (i == uShadowLightIndex)
                    {
                        shadow = ShadowCalculation(fragPos, normal, lightDir) * uShadowStrength;
                    }

                    vec3 diffuse = diff * albedo * radiance;
                    vec3 specular = specularStrength * spec * radiance;
                    lighting += (1.0 - shadow) * (diffuse + specular) * attenuation;
                }

                FragColor = vec4(lighting, 1.0);
            }
            )";
        }
    }

    bool DeferredRenderer::Init(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return false;
        }

        Shutdown();

        auto& graphicsAPI = ArkEngine::GetInstance().GetGraphicsAPI();
        m_geoPassShader = graphicsAPI.CreateShaderProgram(GeometryVertexShader(), GeometryFragmentShader());
        m_lightPassShader = graphicsAPI.CreateShaderProgram(LightingVertexShader(), LightingFragmentShader());
        m_shadowPassShader = graphicsAPI.CreateShaderProgram(ShadowVertexShader(), ShadowFragmentShader());
        m_quad = Mesh::CreatePlane();

        if (!m_geoPassShader || !m_lightPassShader || !m_shadowPassShader || !m_quad)
        {
            Shutdown();
            return false;
        }

        m_width = width;
        m_height = height;
        CreateGBuffer(width, height);
        CreateShadowMap();
        CreateOutputBuffer(width, height);
        CreateWhiteTexture();

        return m_gFBO != 0 && m_shadowFBO != 0 && m_outputFBO != 0;
    }

    void DeferredRenderer::Resize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        if (!m_geoPassShader || !m_lightPassShader || !m_shadowPassShader || !m_quad)
        {
            Init(width, height);
            return;
        }

        m_width = width;
        m_height = height;
        DestroyGBuffer();
        DestroyOutputBuffer();
        CreateGBuffer(width, height);
        CreateOutputBuffer(width, height);
    }

    void DeferredRenderer::Shutdown()
    {
        DestroyGBuffer();
        DestroyOutputBuffer();
        DestroyShadowMap();
        DestroyWhiteTexture();

        m_quad.reset();
        m_geoPassShader.reset();
        m_lightPassShader.reset();
        m_shadowPassShader.reset();
        m_width = 0;
        m_height = 0;
    }

    void DeferredRenderer::Render(
        const std::vector<RenderCommand>& commands,
        const CameraData& cameraData,
        const std::vector<LightData>& lights,
        float shadowStrength)
    {
        if (m_width <= 0 || m_height <= 0 || !m_geoPassShader || !m_lightPassShader || !m_shadowPassShader)
        {
            return;
        }

        int shadowLightIndex = -1;
        for (int i = 0; i < static_cast<int>(lights.size()); ++i)
        {
            if (lights[i].type == 0)
            {
                shadowLightIndex = i;
                break;
            }
        }

        const glm::mat4 lightSpaceMatrix = shadowLightIndex >= 0
            ? ComputeLightSpaceMatrix(lights[shadowLightIndex], cameraData)
            : glm::mat4(1.0f);

        if (shadowLightIndex >= 0)
        {
            glViewport(0, 0, kShadowRes, kShadowRes);
            glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);

            m_shadowPassShader->Bind();
            m_shadowPassShader->SetUniform("uLightSpaceMatrix", lightSpaceMatrix);

            for (const RenderCommand& command : commands)
            {
                if (!command.mesh)
                {
                    continue;
                }

                m_shadowPassShader->SetUniform("uModel", command.modelMatrix);
                command.mesh->Bind();
                command.mesh->Draw();
                command.mesh->Unbind();
            }
        }

        glViewport(0, 0, m_width, m_height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        m_geoPassShader->Bind();
        m_geoPassShader->SetUniform("uView", cameraData.viewMatrix);
        m_geoPassShader->SetUniform("uProjection", cameraData.projectionMatrix);
        m_geoPassShader->SetUniform("baseColorTexture", 0);

        for (const RenderCommand& command : commands)
        {
            if (!command.mesh)
            {
                continue;
            }

            GLuint baseColorTexture = m_whiteTexture;
            bool foundTexture = false;
            if (command.material)
            {
                if (Texture* texture = command.material->GetTextureParam("baseColorTexture"))
                {
                    baseColorTexture = texture->GetID();
                    foundTexture = true;
                }
                else
                {
                    command.material->ForEachTexture([&](Texture* texture)
                        {
                            if (!foundTexture && texture)
                            {
                                baseColorTexture = texture->GetID();
                                foundTexture = true;
                            }
						});
                }
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, baseColorTexture);

            m_geoPassShader->SetUniform("uSpecularStrength", ArkEngine::GetInstance().GetSpecularStrength());
            m_geoPassShader->SetUniform("specularMap", 1);
            Texture* specTex = command.material ? command.material->GetTextureParam("specularMap") : nullptr;
            if (specTex)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, specTex->GetID());
                m_geoPassShader->SetUniform("uHasSpecularMap", 1);
            }
            else
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
                m_geoPassShader->SetUniform("uHasSpecularMap", 0);
            }

            m_geoPassShader->SetUniform("uModel", command.modelMatrix);
            command.mesh->Bind();
            command.mesh->Draw();
            command.mesh->Unbind();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_outputFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        m_lightPassShader->Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        m_lightPassShader->SetUniform("uGPosition", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        m_lightPassShader->SetUniform("uGNormal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
        m_lightPassShader->SetUniform("uGAlbedoSpec", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        m_lightPassShader->SetUniform("uShadowMap", 3);

        m_lightPassShader->SetUniform("uCameraPos", cameraData.position);
        m_lightPassShader->SetUniform("uLightCount", std::min(static_cast<int>(lights.size()), kMaxLights));
        m_lightPassShader->SetUniform("uShadowLightIndex", shadowLightIndex >= kMaxLights ? -1 : shadowLightIndex);
        m_lightPassShader->SetUniform("uHasShadow", shadowLightIndex >= 0 ? 1 : 0);
        m_lightPassShader->SetUniform("uShadowStrength", std::clamp(shadowStrength, 0.0f, 1.0f));
        m_lightPassShader->SetUniform("uLightSpaceMatrix", lightSpaceMatrix);

        const int lightCount = std::min(static_cast<int>(lights.size()), kMaxLights);
        for (int i = 0; i < lightCount; ++i)
        {
            const std::string prefix = "uLights[" + std::to_string(i) + "].";
            m_lightPassShader->SetUniform(prefix + "position", lights[i].position);
            m_lightPassShader->SetUniform(prefix + "color", lights[i].color);
            m_lightPassShader->SetUniform(prefix + "direction", lights[i].direction);
            m_lightPassShader->SetUniform(prefix + "intensity", lights[i].intensity);
            m_lightPassShader->SetUniform(prefix + "range", lights[i].range);
            m_lightPassShader->SetUniform(prefix + "type", lights[i].type);
        }

        m_quad->Bind();
        m_quad->Draw();
        m_quad->Unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    GLuint DeferredRenderer::GetOutputTexture() const
    {
        return m_outputTex;
    }

    GLuint DeferredRenderer::GetOutputFramebuffer() const
    {
        return m_outputFBO;
    }

    GLuint DeferredRenderer::GetShadowMapTexture() const
    {
        return m_shadowMap;
    }

    void DeferredRenderer::CreateGBuffer(int w, int h)
    {
        glGenFramebuffers(1, &m_gFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gFBO);

        glGenTextures(1, &m_gPosition);
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

        glGenTextures(1, &m_gNormal);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

        glGenTextures(1, &m_gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedoSpec, 0);

        const GLuint attachments[3] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };
        glDrawBuffers(3, attachments);

        glGenRenderbuffers(1, &m_gDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_gDepthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gDepthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Logging::Error() << "DeferredRenderer: G-buffer incomplete";
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DeferredRenderer::CreateShadowMap()
    {
        glGenFramebuffers(1, &m_shadowFBO);
        glGenTextures(1, &m_shadowMap);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowRes, kShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Logging::Error() << "DeferredRenderer: shadow framebuffer incomplete";
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DeferredRenderer::CreateOutputBuffer(int w, int h)
    {
        glGenFramebuffers(1, &m_outputFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_outputFBO);

        glGenTextures(1, &m_outputTex);
        glBindTexture(GL_TEXTURE_2D, m_outputTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outputTex, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Logging::Error() << "DeferredRenderer: output framebuffer incomplete";
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DeferredRenderer::CreateWhiteTexture()
    {
        const unsigned char white[] = { 255, 255, 255, 255 };
        glGenTextures(1, &m_whiteTexture);
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void DeferredRenderer::DestroyGBuffer()
    {
        if (m_gFBO) { glDeleteFramebuffers(1, &m_gFBO); m_gFBO = 0; }
        if (m_gPosition) { glDeleteTextures(1, &m_gPosition); m_gPosition = 0; }
        if (m_gNormal) { glDeleteTextures(1, &m_gNormal); m_gNormal = 0; }
        if (m_gAlbedoSpec) { glDeleteTextures(1, &m_gAlbedoSpec); m_gAlbedoSpec = 0; }
        if (m_gDepthRBO) { glDeleteRenderbuffers(1, &m_gDepthRBO); m_gDepthRBO = 0; }
    }

    void DeferredRenderer::DestroyOutputBuffer()
    {
        if (m_outputFBO) { glDeleteFramebuffers(1, &m_outputFBO); m_outputFBO = 0; }
        if (m_outputTex) { glDeleteTextures(1, &m_outputTex); m_outputTex = 0; }
    }

    void DeferredRenderer::DestroyShadowMap()
    {
        if (m_shadowFBO) { glDeleteFramebuffers(1, &m_shadowFBO); m_shadowFBO = 0; }
        if (m_shadowMap) { glDeleteTextures(1, &m_shadowMap); m_shadowMap = 0; }
    }

    void DeferredRenderer::DestroyWhiteTexture()
    {
        if (m_whiteTexture) { glDeleteTextures(1, &m_whiteTexture); m_whiteTexture = 0; }
    }

    glm::mat4 DeferredRenderer::ComputeLightSpaceMatrix(const LightData& light, const CameraData& cam) const
    {
        glm::vec3 lightDir = light.direction;
        if (glm::length(lightDir) < 0.0001f)
        {
            lightDir = glm::vec3(-0.3f, -1.0f, -0.2f);
        }
        lightDir = glm::normalize(lightDir);

        const glm::vec3 center = cam.position;
        const glm::vec3 lightPosition = center - lightDir * 35.0f;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(up, lightDir)) > 0.95f)
        {
            up = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        const glm::mat4 lightView = glm::lookAt(lightPosition, center, up);
        const glm::mat4 lightProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 1.0f, 100.0f);
        return lightProjection * lightView;
    }
}