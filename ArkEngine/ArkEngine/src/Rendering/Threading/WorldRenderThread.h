#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Ark::Rendering
{
	struct WorldRenderInput
	{
		uint32_t width = 1;
		uint32_t height = 1;

		// Demo object for the current viewport (until a real Scene/ECS is wired).
		bool cubeEnabled = true;
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotationDeg{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		glm::vec3 tint{ 1.0f, 1.0f, 1.0f };
	};

	// Renders the "world" (viewport content) on a dedicated thread using a hidden
	// GLFW window + shared OpenGL context. The main thread remains responsible for:
	// - GLFW event polling
	// - ImGui frame lifecycle + rendering
	// - SwapBuffers / presentation
	class WorldRenderThread
	{
	public:
		explicit WorldRenderThread(GLFWwindow* shareWithWindow);
		~WorldRenderThread();

		WorldRenderThread(const WorldRenderThread&) = delete;
		WorldRenderThread& operator=(const WorldRenderThread&) = delete;

		// Submit the latest parameters for the next render.
		// Non-blocking: the render thread will use the most recent input it received.
		void Submit(const WorldRenderInput& input);

		// Shared texture id produced by the render thread (0 until first render).
		uint32_t GetLatestTextureId() const { return m_latestTextureId.load(std::memory_order_relaxed); }

	private:
		void ThreadMain();
		void RequestStop();

	private:
		GLFWwindow* m_renderWindow = nullptr; // hidden shared-context window

		std::thread m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cv;

		WorldRenderInput m_input{};
		bool m_hasInput = false;
		bool m_stop = false;

		std::atomic<uint32_t> m_latestTextureId{ 0 };
	};
}