#include "glad/glad.h"

#include <GLFW/glfw3.h>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#include "ArkWindow.h"
#include <stdexcept>
#include <iostream>


static void EnableDarkTitleBar(GLFWwindow* window)
{
#if defined(_WIN32)
    HWND hwnd = glfwGetWin32Window(window);

    BOOL useDark = TRUE;
    DwmSetWindowAttribute(
        hwnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &useDark,
        sizeof(useDark)
    );
}
#else
	(void)window;
#endif

static void GLFWErrorCallback(int error, const char* description)
{
    std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}

ArkWindow::ArkWindow(int width, int height, const std::string& title)
{
    glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    EnableDarkTitleBar(m_Window);
}

ArkWindow::~ArkWindow()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool ArkWindow::ShouldClose() const
{
    return glfwWindowShouldClose(m_Window);
}

void ArkWindow::PollEvents() const
{
    glfwPollEvents();
}

void ArkWindow::SwapBuffers() const
{
    glfwSwapBuffers(m_Window);
}

void ArkWindow::SetVSync(bool enabled)
{
    glfwSwapInterval(enabled ? 1 : 0);
}

