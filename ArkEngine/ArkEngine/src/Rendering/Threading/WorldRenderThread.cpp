#include "WorldRenderThread.h"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera/Camera.h"
#include "Logger.h"
#include "Meshes/Cube.h"
#include "Rendering/Framebuffer/Framebuffer.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Ark::Rendering
{
	static GLFWwindow* CreateHiddenSharedContextWindow(GLFWwindow* shareWith)
	{
		if (!shareWith)
			return nullptr;

		// Mirror the main window's GL config and create a hidden window sharing objects.
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* w = glfwCreateWindow(1, 1, "ArkRenderThreadContext", nullptr, shareWith);
		return w;
	}

	WorldRenderThread::WorldRenderThread(GLFWwindow* shareWithWindow)
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
			// Must be destroyed before glfwTerminate() (owned by ArkWindow).
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

		// GLAD is process-global. It's already loaded on the main thread after a context
		// is current, but calling it again here is harmless and makes this thread robust.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Logging::Error() << "WorldRenderThread: Failed to initialize GLAD on render thread.\n";
			return;
		}

		glEnable(GL_DEPTH_TEST);

		// Dedicated render-thread resources (avoid relying on objects created on the main context).
		Shader viewportShader;
		if (!viewportShader.LoadFromFiles(
			"ArkEngine/Resources/Shaders/vertex.glsl",
			"ArkEngine/Resources/Shaders/fragment.glsl"))
		{
			Logging::Error() << "WorldRenderThread: Failed to load viewport shader.\n";
			return;
		}

		CubeMesh cubeMesh;
		ArkCamera camera(
			glm::vec3(0.0f, 0.0f, 3.0f),
			45.0f,
			1280.0f / 720.0f,
			0.1f,
			100.0f);

		Framebuffer viewportFramebuffer;

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

			const uint32_t w = (input.width < 1) ? 1u : input.width;
			const uint32_t h = (input.height < 1) ? 1u : input.height;

			// (Re)allocate FBO as needed.
			if (viewportFramebuffer.GetColorTextureId() == 0)
			{
				if (!viewportFramebuffer.Create(w, h))
				{
					Logging::Error() << "WorldRenderThread: Failed to create viewport framebuffer.\n";
					continue;
				}
			}
			else
			{
				if (!viewportFramebuffer.Resize(w, h))
				{
					Logging::Error() << "WorldRenderThread: Failed to resize viewport framebuffer.\n";
					continue;
				}
			}

			viewportFramebuffer.Bind();
			glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera.SetAspect(static_cast<float>(w) / static_cast<float>(h));
			camera.SetPosition(input.camera.position);
			camera.SetRotation(input.camera.pitchYawDeg.x, input.camera.pitchYawDeg.y);
			camera.SetFOV(input.camera.fovDeg);
			camera.SetClipPlanes(input.camera.nearPlane, input.camera.farPlane);

			viewportShader.Bind();
			for (const RenderInstance& inst : input.instances)
			{
				const glm::mat4 mvp = camera.GetViewProjection() * inst.model;
				viewportShader.SetMat4("uMVP", mvp);
				viewportShader.SetVec3("u_Tint", inst.tint);
				cubeMesh.Draw();
			}

			Framebuffer::Unbind();

			// Ensure the texture is fully rendered before the main thread uses it.
			// (Simple correctness-first sync; can be upgraded to GL sync objects later.)
			glFinish();

			m_latestTextureId.store(viewportFramebuffer.GetColorTextureId(), std::memory_order_relaxed);
		}

		glfwMakeContextCurrent(nullptr);
	}
}