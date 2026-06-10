#pragma once

#include "Core/Common.h"

#include <memory>

namespace Engine
{
    class Mesh;
    class ShaderProgram;

    class SkySphereRenderer
    {
    public:
        bool Init();
        void Shutdown();
        void Render(const CameraData& cameraData);

    private:
        std::shared_ptr<Mesh> m_sphere;
        std::shared_ptr<ShaderProgram> m_shader;
    };
}
