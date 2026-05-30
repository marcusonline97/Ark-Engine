#pragma once

#include "Input/InputManager.h"
#include "Graphics/Texture.h"
#include "Graphics/GraphicsAPI.h"
#include "Render/RenderQueue.h"
#include "Scene/Scene.h"
#include "AssetManager/FileSystem.h"
#include "Physics/PhysicsManager.h"
#include "Audio/AudioManager.h"
#include "Font/FontManager.h"
#include "Scene/Components/UI/UIInputSystem.h"

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
		void SetCursorEnabled(bool enabled);
		bool IsEditorUIActive() const;

		static ArkEngine& GetInstance();

		void SetApplication(Application* app);
		Application* GetApplication();

		InputManager& GetInputManager();
		GraphicsAPI& GetGraphicsAPI();
		RenderQueue& GetRenderQueue();
		FileSystem& GetFileSystem();
		TextureManager& GetTextureManager();
		PhysicsManager& GetPhysicsManager();
		AudioManager& GetAudioManager();
		FontManager& GetFontManager();
		UIInputSystem& GetUIInputSystem();

		void SetScene(const std::shared_ptr<Scene>& scene);
		const std::shared_ptr<Scene>& GetScene();

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
		PhysicsManager m_physicsManager;
		AudioManager m_audioManager;
		FontManager m_fontManager;
		UIInputSystem m_uiInputSystem;
		std::shared_ptr<Scene> m_currentScene;
		bool m_editorUIActive = false;
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