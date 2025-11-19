#pragma once

#include "../Window/vkWindow.hpp" //Fix later / * Feeling lazy *
#include "../Core/Engine/Rendering/Shaders/vkPipeline.hpp" //Stupidly long path but i will have to fix later :D 
namespace vk
{
	class StartApp
	{

	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;
	
		void run();
	private:
		VkWindow vkWindow{ WIDTH, HEIGHT, "Ark Engine!" };
		VKPipeline vkPipeline{ "Shaders/simple_shader.vert.spv", "Shaders/simple_shader.frag.spv" };
	};
}