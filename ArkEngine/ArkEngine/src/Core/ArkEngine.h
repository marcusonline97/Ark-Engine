#pragma once

#include "Input/InputManager.h"
#include "Graphics/Texture.h"
#include "Graphics/GraphicsAPI.h"
#include "Render/MeshManager.h"
#include "Render/DeferredRenderer.h"
#include "Render/RenderQueue.h"
#include "Scene/Scene.h"
#include "AssetManager/FileSystem.h"
#include "Physics/PhysicsManager.h"
#include "Audio/AudioManager.h"
#include "Font/FontManager.h"
#include "Scene/Components/UI/UIInputSystem.h"
#include "glad/glad.h"

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
		MeshManager& GetMeshManager();
		PhysicsManager& GetPhysicsManager();
		AudioManager& GetAudioManager();
		FontManager& GetFontManager();
		UIInputSystem& GetUIInputSystem();

		void SetScene(const std::shared_ptr<Scene>& scene);
		const std::shared_ptr<Scene>& GetScene();

		// ── Scene-viewport FBO ──────────────────────────────────
// Returns the OpenGL texture ID that contains the last rendered frame.
// SceneEditor samples this to draw the viewport panel.
		GLuint GetSceneColorTexture() const;
		int    GetSceneViewportWidth()  const { return m_fboWidth; }
		int    GetSceneViewportHeight() const { return m_fboHeight; }

		void SetEditorViewportActive(bool active) { m_editorViewportActive = active; }
		bool IsEditorViewportActive() const        { return m_editorViewportActive; }

		void SetDeferredRenderingEnabled(bool enabled);
		bool IsDeferredRenderingEnabled() const;
		void SetShadowStrength(float strength);
		float GetShadowStrength() const;
		void SetSpecularStrength(float strength);
		float GetSpecularStrength() const;

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
				// ── FBO helpers ─────────────────────────────────────────
		void CreateFBO(int width, int height);
		void ResizeFBO(int width, int height);
		void DestroyFBO();

		std::unique_ptr<Application> m_application;
		std::chrono::steady_clock::time_point m_lastTimePoint;

		GLFWwindow* m_window = nullptr;
		InputManager m_inputManager;
		GraphicsAPI m_graphicsAPI;
		RenderQueue m_renderQueue;
		DeferredRenderer m_deferredRenderer;
		FileSystem m_fileSystem;
		TextureManager m_textureManager;
		MeshManager m_meshManager;
		PhysicsManager m_physicsManager;
		AudioManager m_audioManager;
		FontManager m_fontManager;
		UIInputSystem m_uiInputSystem;
		std::shared_ptr<Scene> m_currentScene;
		bool m_editorUIActive = false;
		bool m_editorViewportActive = false;
		
		// Scene FBO
		GLuint m_fbo = 0;
		GLuint m_fboColorTex = 0;
		GLuint m_fboDepthRBO = 0;
		int    m_fboWidth = 0;
		int    m_fboHeight = 0;
		bool   m_deferredRenderingEnabled = true;
		float  m_shadowStrength = 0.5f;
		float m_specularStrength = 0.5f;
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