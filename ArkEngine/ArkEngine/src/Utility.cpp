#include "Utility.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "Logger.h"

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