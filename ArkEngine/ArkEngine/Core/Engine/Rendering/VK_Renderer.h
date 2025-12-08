#pragma once

#define GLFW_INCLUDE_VULKAN //-> this is just a glfw function to include the default vulkan header.
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

class VK_Renderer
{
public:

	VK_Renderer();
	~VK_Renderer();
};