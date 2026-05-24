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

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto& inputManager = Engine::ArkEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetMouseButtonPressed(button, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetMouseButtonPressed(button, false);
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

		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwMakeContextCurrent(m_window);

        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
		}

		m_graphicsAPI.Init();
		m_physicsManager.Init();
		m_audioManager.Init();
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

            m_application->Update(deltaTime);

            m_graphicsAPI.SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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
						cameraData.position = cameraObject->GetWorldPosition();
                    }
                }

                lights = m_currentScene->CollectLights();
            }

            m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);

            glfwSwapBuffers(m_window);

			m_inputManager.SetMousePositionChanged(false);
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



    void ArkEngine::SetScene(Scene* scene)
    {
        m_currentScene.reset(scene);
    }

    Scene* ArkEngine::GetScene()
    {
        return m_currentScene.get();
    }
}