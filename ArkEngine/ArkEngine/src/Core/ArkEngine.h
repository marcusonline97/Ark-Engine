#pragma once

#include "Input/InputManager.h"

#include <memory>
#include <chrono>


struct GLFWwindow;

namespace Engine
{
	class Application;

	class ArkEngine
	{
	public:

		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

	
		//-------------------------------------------
		// Functions
		//-------------------------------------------

		bool Init(int width, int height);

		void Run();

		void Destroy();

		static ArkEngine& GetInstance();

		void SetApplication(Application* app);
		Application* GetApplication();

		InputManager& GetInputManager();

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::unique_ptr<Application> m_application;
		std::chrono::steady_clock::time_point m_lastTimePoint;

		GLFWwindow* m_window = nullptr;
		InputManager m_inputManager;
		//-------------------------------------------
		// Functions
		//-------------------------------------------
		ArkEngine() = default;
		ArkEngine(const ArkEngine&) = delete;
		ArkEngine(ArkEngine&&) = delete;
		ArkEngine& operator=(const ArkEngine&) = delete;
		ArkEngine& operator=(ArkEngine&&) = delete;
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	};
}