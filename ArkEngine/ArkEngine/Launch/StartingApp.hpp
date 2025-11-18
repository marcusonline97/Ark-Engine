#pragma once

#include "../Window/vkWindow.hpp" //Fix later / * Feeling lazy *

namespace vk
{
	class StartApp
	{

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
	
		void run();
	private:
		VkWindow wkWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };

	};
}