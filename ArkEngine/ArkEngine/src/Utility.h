#pragma once
#include <string>
struct GLFWwindow;

namespace Util
{
    // Sets the GLFW window icon from a PNG file.
    // Supports passing multiple sizes later if needed.
    void SetWindowIcon(GLFWwindow* window, const std::string& iconPath);
}