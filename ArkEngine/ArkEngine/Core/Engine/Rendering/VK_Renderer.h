#pragma once

#define GLFW_INCLUDE_VULKAN //-> this is just a glfw function to include the default vulkan header.
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include "../ArkEngine/Core/Engine/Utilities/Utilities.h" // <- Man fuck this Pathing
class VK_Renderer
{
public:

	VK_Renderer();

	int Init(GLFWwindow * newWindow);

	void CleanUp();

	~VK_Renderer();


private:
	GLFWwindow* window;


	// Vulkan Components  /-> most things named Vk is usually a Vulkan thing 
	VkInstance instance;

	struct
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	}
	mainDevice;

	VkQueue graphicQueue;

	// Vulkan Functions
	// - Create Functions
	void createInstance();
	void createLogicalDevice();
	// - Get Functions
	void getPhysicalDevice();


	// - Support Functions
	// 
	// - Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceSuitable(VkPhysicalDevice device);


	// - Getter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
};