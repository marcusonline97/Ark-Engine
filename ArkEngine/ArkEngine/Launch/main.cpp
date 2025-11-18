#include "StartingApp.hpp"

//std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
int main()
{
	vk::StartApp app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}



/*
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <iostream>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

GLFWwindow* window = NULL;

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void CheckInstanceExtensionProperties()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extension supported\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;
}

int main(int argc, char* argv[])
{
	if (!glfwInit())
	{
		return 1;
	}

	if (!glfwVulkanSupported())
	{
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ark Engine version 0.1", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	CheckInstanceExtensionProperties();

	glfwSetKeyCallback(window, GLFW_KeyCallback);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
}
*/