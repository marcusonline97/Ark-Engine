#include "StartingApp.hpp"

namespace vk
{
	void StartApp::run()
	{
		while (!vkWindow.shouldClose())
		{
			glfwPollEvents();
		}
	}
}