#pragma once

#define GLFW_INCLUDE_VULKAN //-> this is just a glfw function to include the default vulkan header.
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

class VK_Renderer
{
public:

	VK_Renderer();

	int Init(GLFWwindow * newWindow);

	~VK_Renderer();


private:
	GLFWwindow* window;


	// Vulkan Components  /-> most things named Vk is usually a Vulkan thing 
	VkInstance instance;


	// Vulkan Functions
	// - Create Functions
	void createInstance();

	// - Support Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);

};