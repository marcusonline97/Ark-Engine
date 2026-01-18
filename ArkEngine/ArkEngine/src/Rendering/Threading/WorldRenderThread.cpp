#include "WorldRenderThread.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Logger.h"
#include "AssetManager/ResourceLoader.h"
#include "Rendering/WorldRendererLegacy.h"

namespace Ark::Rendering
{
	static GLFWwindow* CreateHiddenSharedContextWindow(GLFWwindow* shareWith)
	{
		if (!shareWith)
			return nullptr;

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* w = glfwCreateWindow(1, 1, "ArkRenderThreadContext", nullptr, shareWith);
		return w;
	}

	WorldRenderThread::WorldRenderThread(GLFWwindow* shareWithWindow, ResourceLoader* cpuLoader)
		: m_cpuLoader(cpuLoader)
	{
		m_renderWindow = CreateHiddenSharedContextWindow(shareWithWindow);
		if (!m_renderWindow)
			throw std::runtime_error("WorldRenderThread: failed to create shared OpenGL context window");

		m_thread = std::thread([this]() { ThreadMain(); });
	}

	WorldRenderThread::~WorldRenderThread()
	{
		RequestStop();
		if (m_thread.joinable())
			m_thread.join();

		if (m_renderWindow)
		{
			glfwDestroyWindow(m_renderWindow);
			m_renderWindow = nullptr;
		}
	}

	void WorldRenderThread::RequestStop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
			m_hasInput = true;
		}
		m_cv.notify_one();
	}

	void WorldRenderThread::Submit(const WorldRenderInput& input)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_input = input;
			m_hasInput = true;
		}
		m_cv.notify_one();
	}

	void WorldRenderThread::ThreadMain()
	{
		glfwMakeContextCurrent(m_renderWindow);
		glfwSwapInterval(0);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Logging::Error() << "WorldRenderThread: Failed to initialize GLAD on render thread.\n";
			return;
		}

		glEnable(GL_DEPTH_TEST);

		WorldRendererLegacy renderer;

		std::unordered_map<uint32_t, float> animTimeByObjectId;
		animTimeByObjectId.reserve(256);

		double lastTime = glfwGetTime();

		while (true)
		{
			WorldRenderInput input{};
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_cv.wait(lock, [this]() { return m_hasInput; });
				m_hasInput = false;
				if (m_stop)
					break;
				input = m_input;
			}

			const double now = glfwGetTime();
			const float dt = static_cast<float>(now - lastTime);
			lastTime = now;

			// Update per-object animation time (skeletal only).
			for (auto& inst : input.instances)
			{
				if (inst.meshType != RenderMeshType::Skeletal)
					continue;

				if (inst.objectId == 0)
					continue;

				float& t = animTimeByObjectId[inst.objectId];
				t += dt;
				inst.animTimeSec = t;
			}

			renderer.Render(input);
			m_latestTextureId.store(renderer.GetOutputTextureId(), std::memory_order_relaxed);
			m_latestTriangleCount.store(renderer.GetLastTriangleCount(), std::memory_order_relaxed);

			glFinish();
		}

		glfwMakeContextCurrent(nullptr);
	}
}