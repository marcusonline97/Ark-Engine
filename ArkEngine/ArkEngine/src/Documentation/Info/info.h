
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



//INFO DUMP ___________________________________________

/*

Adding extensions:
Swap out images in a double/triple buffer swapping out them when appropriate using a swapchain

Swapchain is: another extension, will need to be checked for compatibility, it's a group of images that can be drawn to and presented.
the objest is set up so it can be queried to get a new image to be drawn to and then be presented.
This requires a lot of synchronisation. -> To do
Swapchain has 3 major parts surface:
surface Capability :what it is capable of handling such as image sizes
surface Format : the format of the surfaec of our Swapchain will be present to (e.g RGB)
presentation mode : The order and timing of images being presented to the image.

There are 4 presentation modes and 2 of them introduce "tearing"

VK_PRESENT_MODE_IMMEDIATE_KHR : in this mode, the surface image will be replicared immediately after the next image to be displayed has finished rendering.
This can be an issue as our surface may only be halfway through drawing to the screen when we replace it, causing the second half to be a different image, this results in "tearing".
//Likely good if you wanna force image rendering at the cost of chances of tearing

VK_PRESENT_MODE_MAILBOX_KHR : images ready to present are added to a queue of size 1. The Surface uses this image only at a Vertical Blank to avoid tearing.
If a new image is sent to the queue to be presented, whatever is currently in the queue is discarded and becomes a vacant image ready to be drawn to again.
// This makes this very ideal to avoid screen and frame tearing.

VK_PRESENT_MODE_FIFO_KHR: Images ready to present are added to a queue of a certain size. They are removed one at a time at each vertical blank, so there is no tearing.
If the queue is full, the program will wait.
//Basicly the same as the mailbox just not a queue of size one.

VK_PRESENT_MODE_FIFO_RELAXED_KHR : this mode works in the same manor as FIFO mode, except for the situation where there are no images on the queue during a Vertical Blank.
If the presentation queue is empty during a vertical blank, it will act as Immediate mode.
The next image placed on to the presentation queue will no longer wait for a Vertical Blank.
It will instead be passed straight to the surface, like in Immediate mode. This will cause tearing.
after it returns to FIFO Mode.
//Note we do not like tearing :3

Images and image views:
When the swapchain is created, it will automatically create a set of images to be used by the swapchain.
Since we will be drawing to them, we need access to them. To do this, the swapchain can be queried and an array of the images can be returned.

However, these images can't be used directly since they are just the raw image.

And "ImageView" type can be created to interface with an image.
an ImageView simply describes how to read an image (e.g 2D or 3D adresses, format, etc) and what part of the image to read(colour channel, mip level, etc)





At any one time, there will be one image present at the Surface that is being drawn to the screen

When the image completes, the screen is then cleared to start drawing again, the period of time AFTER this clear, and BEFORE it starts drawing again,
is known as a "Vertical Blank Interval" or simply a "Vertical Blank". This is usually the best time to replace the Surface image.


Presentation Queue is a queue that handles present operations. Graphics queues usually have the required feature.
This means our Graphics Queue and Presentation Queue will usually be the same queue.


*/