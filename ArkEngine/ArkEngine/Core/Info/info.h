
//This is a Read Only header with just useful info for myself to read up on


/*
#define GLM_FORCE_RADIANS //vulkan specific
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> //It seems like my vulkan SDK actually come prepacked with glm for some reason
//And my setup seems to wanna link to both my Vulkan SDK and my local GLM inside of the project so be mindful of the pathing
*/

// Tons of shit will break and require a lot of try catching, logging and ensuring things work. Ofcourse it will 98% of the time
// compile but there may be a likely hood that something could occur during runtime. So running a lot of print logs and
// error catching is essential to make vulkan work.


/* Generic Good to know shit 
Validation layers add safety nets
They hook into Vulkan calls and verify that parameters, memory usage, and synchronization are correct



A logical device is a software abstraction you create from a physical device.
It’s created with vkCreateDevice, where you specify:

Which queue families you want to use (e.g., graphics + presentation).

Which features you want enabled (e.g., anisotropic filtering).

Which extensions you need (e.g., VK_KHR_swapchain).

//--------------------------------------------------//

A physical device represents an actual GPU (graphics card) or other hardware accelerator present in your system.

It’s discovered through the Vulkan instance with vkEnumeratePhysicalDevices.

It exposes:

Hardware properties (vendor ID, device name, supported features).

Queue families (graphics, compute, transfer).

Memory types (VRAM, system RAM).

Supported extensions (e.g., ray tracing, swapchain).
*/