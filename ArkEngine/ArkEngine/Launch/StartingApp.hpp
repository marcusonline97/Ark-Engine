#pragma once

#include "../Window/vkWindow.hpp" //Fix later / * Feeling lazy *

namespace vk
{
	class StartApp
	{

	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;
	
		void run();
	private:
		VkWindow wkWindow{ WIDTH, HEIGHT, "Ark Engine!" };

	};
}