#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <cassert>

namespace vk
{
	class VkWindow
	{

	public:
		VkWindow(int w, int h, std::string name);
		~VkWindow();

		VkWindow(const VkWindow&) = delete;
		VkWindow& operator=(const VkWindow&) = delete;


		bool shouldClose() { return glfwWindowShouldClose(window); }

	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;

		GLFWwindow* window;

	};
}