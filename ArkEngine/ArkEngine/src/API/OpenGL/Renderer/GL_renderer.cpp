#include "GL_renderer.h"
#include "../../Renderer/Renderer.h"

namespace OpenGLRenderer {
    void Init() {
        // OpenGL-specific renderer initialization
        // This can be expanded to include OpenGL-specific setup
        Renderer::Init();
    }

    void RenderFrame() {
        Renderer::RenderFrame();
    }

    void RenderEditorFrame() {
        Renderer::RenderEditorFrame();
    }

    void Cleanup() {
        // OpenGL-specific cleanup
    }
}
