#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS //vulkan specific
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> //It seems like my vulkan SDK actually come prepacked with glm for some reason
//And my setup seems to wanna link to both my Vulkan SDK and my local GLM inside of the project so be mindful of the pathing

#include <glm/mat4x4.hpp> 

#include <iostream>

int main()
{
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 670, "Ark Engine", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	printf("Extension Count: %u\n", extensionCount); // switch out with custom logger later

	glm::mat4 testMatrix(1.0f);
	glm::vec4 testVector(1.0f);

	auto testResult = testMatrix * testVector;


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	
	glfwTerminate();

	return 0;
}

