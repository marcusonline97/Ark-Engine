#include "vkWindow.hpp"

namespace vk
{
	VkWindow::VkWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
	{
		initWindow();
	}

	VkWindow::~VkWindow()
	{
        glfwDestroyWindow(window);

        glfwTerminate();
	}

	void VkWindow::initWindow()
	{
        if (!glfwInit())
        {
            // Initialization failed
            assert(false && "Failed to initialize GLFW");
            return; // or throw an exception if you prefer
        }

        if (!glfwVulkanSupported())
        {
            assert(false && "Vulkan not supported by GLFW");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        if (!window)
        {
            assert(false && "Failed to create GLFW window");
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
	}
}