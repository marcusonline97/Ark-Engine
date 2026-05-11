#include "ArkEngine.h"
#include "Application.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
	void ArkEngine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* engine = static_cast<ArkEngine*>(glfwGetWindowUserPointer(window));
		if (!engine) return;

		if (action == GLFW_PRESS)
			engine->GetInputManager().SetKeyPressed(key, true);
		else if (action == GLFW_RELEASE)
			engine->GetInputManager().SetKeyPressed(key, false);
	}

	bool ArkEngine::Init(int width, int height)
	{
		// Initialization code here
		if (!m_application)
		{
			return false;
		}

		if (!glfwInit())
		{
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(width, height, "ArkEngine", nullptr, nullptr);

		if (m_window == nullptr)
		{
			std::cout << "Error creating window " << std::endl;
			glfwTerminate();
			return false;
		}

		glfwSetWindowUserPointer(m_window, this);

		glfwSetKeyCallback(m_window, ArkEngine::KeyCallback);

		glfwMakeContextCurrent(m_window);

		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Error initializing GLAD " << std::endl;
			glfwTerminate();
			return false;
		}


		return m_application->Init();
	}

	void ArkEngine::Run()
	{
		// Main loop code here
		if (!m_application)
		{
			return;
		}
		m_lastTimePoint = std::chrono::high_resolution_clock::now();

		while (!glfwWindowShouldClose(m_window) && !m_application->NeedsToBeClosed())
		{

			glfwPollEvents();

			auto now = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();

			m_lastTimePoint = now;

			m_application->Update(deltaTime);

			glfwSwapBuffers(m_window);
		}
	}

	void ArkEngine::Destroy()
	{
		if (m_application)
		{
			m_application->Destroy();
			m_application.reset();
			glfwTerminate();
			m_window = nullptr;
		}
		// Cleanup code here
	}

	ArkEngine& ArkEngine::GetInstance()
	{
		static ArkEngine instance;
		return instance;
	}

	void ArkEngine::SetApplication(Application* app)
	{
		m_application.reset(app);
	}

	Application* ArkEngine::GetApplication()
	{
		return m_application.get();
	}
	InputManager& ArkEngine::GetInputManager()
	{
		return m_inputManager;
	}

}