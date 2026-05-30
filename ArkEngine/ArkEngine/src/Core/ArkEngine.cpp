#include "ArkEngine.h"
#include "Application.h"
#include "Scene/GameObject.h"
#include "Scene/Component.h"
#include "Scene/Components/CameraComponent.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace Engine
{
    void keyCallback(GLFWwindow* window, int key, int, int action, int)
    {
        auto& inputManager = Engine::ArkEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetKeyPressed(key, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetKeyPressed(key, false);
        }
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
    {
        auto& inputManager = Engine::ArkEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetMouseButtonPressed(button, true);
            inputManager.SetMouseButtonWasPressed(button, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetMouseButtonPressed(button, false);
            inputManager.SetMouseButtonWasReleased(button, true);
        }
    }

    void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto& inputManager = Engine::ArkEngine::GetInstance().GetInputManager();

        inputManager.SetMousePositionOld(inputManager.GetMousePositionCurrent());

        glm::vec2 currentPos(static_cast<float>(xpos), static_cast<float>(ypos));
        inputManager.SetMousePositionCurrent(currentPos);

        inputManager.SetMousePositionChanged(true);
    }

	void windowSizeCallback(GLFWwindow* window, int width, int height)
    {
		Engine::ArkEngine::GetInstance().GetGraphicsAPI().SetViewport(0, 0, width, height);
    }

    ArkEngine& ArkEngine::GetInstance()
    {
        static ArkEngine instance;
        return instance;
    }

    bool ArkEngine::Init(int width, int height)
    {
        if (!m_application)
        {
            return false;
        }

        Scene::RegisterTypes();
        m_application->RegisterTypes();

#if defined (__linux__)
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

        if (!glfwInit())
        {
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, "Ark Engine", nullptr, nullptr);

        if (m_window == nullptr)
        {
            std::cout << "Error creating window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(m_window, keyCallback);
		glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetCursorPosCallback(m_window, cursorPositionCallback);
		glfwSetWindowSizeCallback(m_window, windowSizeCallback);

        glfwMakeContextCurrent(m_window);

        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
		}

		m_graphicsAPI.Init();
        m_graphicsAPI.SetViewport(0, 0, width, height);
		m_physicsManager.Init();
		m_audioManager.Init();
        m_renderQueue.Init();
		m_fontManager.Init();  
        return m_application->Init();
    }

    void ArkEngine::Run()
    {
        if (!m_application)
        {
            return;
        }

        m_lastTimePoint = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(m_window) && !m_application->NeedsToBeClosed())
        {
            glfwPollEvents();

            auto now = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
            m_lastTimePoint = now;

            m_physicsManager.Update(deltaTime);

            if (m_uiInputSystem.IsActive())
            {
                m_uiInputSystem.Update(deltaTime);
            }

            m_application->Update(deltaTime);

            m_graphicsAPI.ClearBuffers();

            CameraData cameraData;
            std::vector<LightData> lights;

            int width = 0;
            int height = 0;
            glfwGetWindowSize(m_window, &width, &height);
            float aspect = static_cast<float>(width) / static_cast<float>(height);

            if (m_currentScene)
            {
                if (auto cameraObject = m_currentScene->GetMainCamera())
                {
                    // logic for matrices
                    auto cameraComponent = cameraObject->GetComponent<CameraComponent>();
                    if (cameraComponent)
                    {
                        cameraData.viewMatrix = cameraComponent->GetViewMatrix();
                        cameraData.projectionMatrix = cameraComponent->GetProjectionMatrix(aspect);
                        cameraData.orthoMatrix = glm::ortho(0.0f, static_cast<float>(width),0.0f, static_cast<float>(height));
						cameraData.position = cameraObject->GetWorldPosition();
                    }
                }

                lights = m_currentScene->CollectLights();
            }

            m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);

            glfwSwapBuffers(m_window);

			m_inputManager.ClearStates();
        }

        m_application.reset(nullptr); // Ensure to clean the memory out of the game
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
    }

    void ArkEngine::SetCursorEnabled(bool enabled)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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

    GraphicsAPI& ArkEngine::GetGraphicsAPI()
    {
        return m_graphicsAPI;
    }

    RenderQueue& ArkEngine::GetRenderQueue()
    {
        return m_renderQueue;
    }

    FileSystem& ArkEngine::GetFileSystem()
    {
        return m_fileSystem;
    }

    TextureManager& ArkEngine::GetTextureManager()
    {
        return m_textureManager;
    }

    PhysicsManager& ArkEngine::GetPhysicsManager()
    {
        return m_physicsManager;
    }

    AudioManager& ArkEngine::GetAudioManager()
    {
		return m_audioManager;
    }

    FontManager& ArkEngine::GetFontManager()
    {
		return m_fontManager;
    }

    UIInputSystem& ArkEngine::GetUIInputSystem()
    {
        return m_uiInputSystem;
    }

    void ArkEngine::SetScene(const std::shared_ptr<Scene>& scene)
    {
        m_currentScene = scene;
    }

    const std::shared_ptr<Scene>& ArkEngine::GetScene()
    {
        return m_currentScene;
    }
}