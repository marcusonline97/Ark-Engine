#pragma once

#include "Core/Common.h"
#include "Render/RenderQueue.h"

#include <GLAD/glad.h>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

namespace Engine
{
    class Mesh;
    class ShaderProgram;

    class DeferredRenderer
    {
    public:
        bool Init(int width, int height);
        void Resize(int width, int height);
        void Shutdown();

        void Render(
            const std::vector<RenderCommand>& commands,
            const CameraData& cameraData,
            const std::vector<LightData>& lights,
            float shadowStrength);

        GLuint GetOutputTexture() const;
        GLuint GetOutputFramebuffer() const;
        GLuint GetShadowMapTexture() const;

    private:
        GLuint m_gFBO = 0;
        GLuint m_gPosition = 0;
        GLuint m_gNormal = 0;
        GLuint m_gAlbedoSpec = 0;
        GLuint m_gDepthRBO = 0;

        GLuint m_shadowFBO = 0;
        GLuint m_shadowMap = 0;
        static constexpr int kShadowRes = 2048;

        GLuint m_outputFBO = 0;
        GLuint m_outputTex = 0;
        GLuint m_whiteTexture = 0;

        int m_width = 0;
        int m_height = 0;

        std::shared_ptr<Mesh> m_quad;
        std::shared_ptr<ShaderProgram> m_geoPassShader;
        std::shared_ptr<ShaderProgram> m_lightPassShader;
        std::shared_ptr<ShaderProgram> m_shadowPassShader;

        void CreateGBuffer(int w, int h);
        void CreateShadowMap();
        void CreateOutputBuffer(int w, int h);
        void CreateWhiteTexture();
        void DestroyGBuffer();
        void DestroyOutputBuffer();
        void DestroyShadowMap();
        void DestroyWhiteTexture();
        glm::mat4 ComputeLightSpaceMatrix(const LightData& light, const CameraData& cam) const;
    };
}
