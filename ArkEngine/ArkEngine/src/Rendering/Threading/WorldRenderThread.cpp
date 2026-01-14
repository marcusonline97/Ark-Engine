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

			const float aspect = static_cast<float>(w) / static_cast<float>(h);
			ArkCamera camera(
				input.camera.position,
				input.camera.fov,
				aspect,
				input.camera.nearPlane,
				input.camera.farPlane);
			camera.SetRotation(input.camera.rotationDeg.x, input.camera.rotationDeg.y);

			for (const auto& obj : input.objects)
			{
				if (!obj.enabled)
					continue;

				const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
				const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
				const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
				const glm::mat4 rot = rotZ * rotY * rotX;
				const glm::mat4 model =
					glm::translate(glm::mat4(1.0f), obj.position) *
					rot *
					glm::scale(glm::mat4(1.0f), obj.scale);

				const glm::mat4 mvp = camera.GetViewProjection() * model;

				viewportShader.Bind();
				viewportShader.SetMat4("uMVP", mvp);
				viewportShader.SetVec3("u_Tint", obj.tint);
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