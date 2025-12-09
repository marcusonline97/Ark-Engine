#include "VK_Renderer.h"

VK_Renderer::VK_Renderer()
{

}

int VK_Renderer::Init(GLFWwindow* newWindow)
{
	window = newWindow;

	try
	{
		createInstance();
	}
	catch (const std::runtime_error &e)
	{
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}



	return 0;
}

VK_Renderer::~VK_Renderer()
{
}

void VK_Renderer::createInstance()
{
	//Information about the application itself - Update version and data? Who cares no one will read this right?
	//Most data wont affect the program itself, more so just for developers to follow up
	//this is just a struct about our App.
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "ArkEngine"; // The Applications name
	appInfo.applicationVersion = VK_MAKE_VERSION(0,1,1); //Current App Version
	appInfo.pEngineName = "ArkEngine"; // Engine Name
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0); // Custom Engine Version
	appInfo.apiVersion = VK_API_VERSION_1_4; // My Vulkan Version


	// Creation Information for a VkInstance (Vulkan Instance)
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo; // 
	createInfo.enabledExtensionCount;
//	createInfo.pNext = //Can help add more detail to extended info where extra data is
//	createInfo.flags = VK_WHATEVER |  Might add flags later i dunno

	// Create list to hold instance extensions
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	//Setup extensions instance will use
	uint32_t glfwExtensionCount = 0;  //GLFW may require multiple extensions
	const char** glfwExtensions;  //Extensions passed as array of cstrings, so need pointer (the array) to pointer {the string}

	// Get GLFW extensions 
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();


	//TODO: Set up Validation Layers
	createInfo.enabledLayerCount = 0; //Enables Validation -> You can actually divide validation for certain layers.
	createInfo.ppEnabledLayerNames = nullptr;

	//Create Instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);// We add our createInfo and add our instance here. 
	//Allocallback isnt added yet but will probably add later at some point
	// Definitly worth reading the VkResult enum... How did they make such a huge Graphics API. It's crazy...
	
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vulkan Instance!");
	}
}

bool VK_Renderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


}
