#define GLFW_INCLUDE_VULKAN //-> this is just a glfw function to include the default vulkan header.
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>



#include "../Engine/Rendering/VK_Renderer.h" // Fix this shit later bruh 

GLFWwindow* window;
VK_Renderer vulkanRenderer;

void InitWindow(std::string wName = "Test Window", const int width = 900, const int height = 750)
{
	//Init GLFW
	glfwInit();

	//Set GLFW To not work with OPENGL by default
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main()
{
	//Create Window 
	InitWindow("Test Window", 900, 800);

	//Create Vulkan Renderer Instance
	if (vulkanRenderer.Init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	//Loop until closed
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	//Destroy glfwWindow and stop GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;

}

