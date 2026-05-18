#pragma once

#include "Input/InputManager.h"
#include "Graphics/Texture.h"
#include "Graphics/GraphicsAPI.h"
#include "Render/RenderQueue.h"
#include "Scene/Scene.h"
#include "AssetManager/FileSystem.h"

#include <chrono>
#include <memory>

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
		GraphicsAPI& GetGraphicsAPI();
		RenderQueue& GetRenderQueue();
		FileSystem& GetFileSystem();
		TextureManager& GetTextureManager();


		void SetScene(Scene* scene);
		Scene* GetScene();

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::unique_ptr<Application> m_application;
		std::chrono::steady_clock::time_point m_lastTimePoint;

		GLFWwindow* m_window = nullptr;
		InputManager m_inputManager;
		GraphicsAPI m_graphicsAPI;
		RenderQueue m_renderQueue;
		FileSystem m_fileSystem;
		TextureManager m_textureManager;
		std::unique_ptr<Scene> m_currentScene;
		//-------------------------------------------
		// Functions
		//-------------------------------------------
		ArkEngine() = default;
		ArkEngine(const ArkEngine&) = delete;
		ArkEngine(ArkEngine&&) = delete;
		ArkEngine& operator=(const ArkEngine&) = delete;
		ArkEngine& operator=(ArkEngine&&) = delete;
	};
}