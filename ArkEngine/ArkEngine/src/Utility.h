#pragma once
#include <string>
struct GLFWwindow;

class Utilities
{
public:
    // Call once per viewport frame rendered.
    // Returns a rolling-average FPS updated ~4 times per second.
    static float TickViewportFPS(double nowSeconds);

    // Last computed viewport FPS.
    static float GetViewportFPS();
};

namespace Util
{
    // Sets the GLFW window icon from a PNG file.
    // Supports passing multiple sizes later if needed.
    void SetWindowIcon(GLFWwindow* window, const std::string& iconPath);
}