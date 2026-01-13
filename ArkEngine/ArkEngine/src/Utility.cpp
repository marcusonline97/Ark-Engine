#include "Utility.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "Logger.h"


namespace
{
    double g_viewportFpsLastSampleTime = 0.0;
    int g_viewportFpsFrames = 0;
    float g_viewportFps = 0.0f;
}

float Utilities::TickViewportFPS(double nowSeconds)
{
    // Rolling average: recompute every ~0.25s for a stable readout.
    if (g_viewportFpsLastSampleTime <= 0.0)
        g_viewportFpsLastSampleTime = nowSeconds;

    ++g_viewportFpsFrames;
    const double elapsed = nowSeconds - g_viewportFpsLastSampleTime;
    if (elapsed >= 0.25)
    {
        g_viewportFps = static_cast<float>(static_cast<double>(g_viewportFpsFrames) / elapsed);
        g_viewportFpsFrames = 0;
        g_viewportFpsLastSampleTime = nowSeconds;
    }

    return g_viewportFps;
}

float Utilities::GetViewportFPS()
{
    return g_viewportFps;
}

namespace Util
{
    void SetWindowIcon(GLFWwindow* window, const std::string& iconPath)
    {
        if (!window || iconPath.empty())
            return;

        int w = 0, h = 0, channels = 0;
        stbi_uc* pixels = stbi_load(iconPath.c_str(), &w, &h, &channels, 4);
        if (!pixels)
        {
            Logging::Warning() << "Failed to load icon: " << iconPath << "\n";
            return;
        }

        GLFWimage image{};
        image.width = w;
        image.height = h;
        image.pixels = pixels;

        glfwSetWindowIcon(window, 1, &image);
        stbi_image_free(pixels);
    }
}