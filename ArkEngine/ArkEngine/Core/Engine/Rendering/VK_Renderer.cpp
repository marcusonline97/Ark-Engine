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
		getPhysicalDevice(); // Requires the creation of an instance.
		createLogicalDevice(); //Logical Device relies entirely on the physical Device
	}
	catch (const std::runtime_error &e)
	{
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}



	return 0;
}

void VK_Renderer::CleanUp()
{
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr); // <- if we make a de-allocator put it in the second argument here
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
	appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1); //Current App Version
	appInfo.pEngineName = "ArkEngine"; // Engine Name
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1); // Custom Engine Version
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

	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("VkInstance does not support required extensions!");
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

void VK_Renderer::createLogicalDevice()
{
	// Get the queue family indices for the chosen Physical Device
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice); // <- should work if the physical device compiles properly

	//Queue the logical device needs to create and info to do so (only 1 for now, will add more later)
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; //index of the family to create a queue from 
	queueCreateInfo.queueCount = 1; // number of queues to create
	float priority = 1.0f;
	queueCreateInfo.pQueuePriorities = &priority; // Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest priority)



	// Information to create our logical device (logical device are sometimes called "Device) Physical are strictly physical
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
	deviceCreateInfo.queueCreateInfoCount = 1; //Number of queue create infos
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // List of queue create infos so device can create required
	deviceCreateInfo.enabledExtensionCount = 0; // Number of enabled Logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = nullptr; // List of enabled logical device extensions;

	//Physical Device Features the Logical Device will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};


	deviceCreateInfo.pEnabledFeatures = &deviceFeatures; // Physical Device Features Logical Device will use

	//Create the logical device for the given physical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice); // <- store it in our logical device
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Logical Device!");
	}

	// Queues are created at the same time as the device..
	// So we want to handle to queues
	// Given Logical Device, of given queue family, of given queue index (0 since only one queue) , place reference in given VkQueue
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicQueue);
}

void VK_Renderer::getPhysicalDevice()
{
	// Enumerate Physical device the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// If no device available, then none support Vulkan!
	if (deviceCount == 0)
	{
		throw std::runtime_error("Can't find GPUs that can support the Vulkan Instance!");
	}

	//Get List of Physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto& device : deviceList)
	{
		if (checkDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}
}

bool VK_Renderer::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	//Keep in mind that instances have extensions and devices also have properties so keep them different


	// Create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtensions : *checkExtensions)
	{
		bool hasExtension = false;

		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtensions, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VK_Renderer::checkDeviceSuitable(VkPhysicalDevice device)
{
	/*
	// Information about the device itself (ID, name, type, vendor, etc
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties); // <- Checkout vkGetPhysicalDeviceProperties

	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	*/ 

	QueueFamilyIndices indices = getQueueFamilies(device);

	return indices.isValid();
}

QueueFamilyIndices VK_Renderer::getQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	//Get all Queue Family Property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	// Go through each queue family and check if it has at least 1 of the requried types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{
		//First check if queue family has at least 1 queue in that family (could have no queues)
		//queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_ BIT to check if it has required 
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) // queue flags can support multiple queue flags
		{
			indices.graphicsFamily = i; // If queuefamily is valid, then get index.

		}
		//Check if queue family indices are in a valid state, stop searching if so
		if (indices.isValid())
		{
			break;
		}

		i++;
	}


	return indices;
}


