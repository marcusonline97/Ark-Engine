#pragma once
#include <string>

struct GLFWwindow;

class ArkWindow
{
public:
    ArkWindow(int width, int height, const std::string& title);
    ~ArkWindow();

    bool ShouldClose() const;
    void PollEvents() const;
    void SwapBuffers() const;

    void SetVSync(bool enabled);

    GLFWwindow* GetNativeHandle() const { return m_Window; }

private:
    GLFWwindow* m_Window = nullptr;
};
