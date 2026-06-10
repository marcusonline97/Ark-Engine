#include "ArkEngine.h"
#include "Application.h"
#include "Scene/GameObject.h"
#include "Scene/Component.h"
#include "Scene/Components/CameraComponent.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <algorithm>
#include <iostream>

namespace Engine
{
    // ── GLFW callbacks ───────────────────────────────────────────────────

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

        auto& inputManager = ArkEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)   inputManager.SetKeyPressed(key, true);
        else if (action == GLFW_RELEASE) inputManager.SetKeyPressed(key, false);
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        auto& im = ArkEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            im.SetMouseButtonPressed(button, true);
            im.SetMouseButtonWasPressed(button, true);
        }
        else if (action == GLFW_RELEASE)
        {
            im.SetMouseButtonPressed(button, false);
            im.SetMouseButtonWasReleased(button, true);
        }
    }

    void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        auto& im = ArkEngine::GetInstance().GetInputManager();
        im.SetMousePositionOld(im.GetMousePositionCurrent());
        im.SetMousePositionCurrent(glm::vec2((float)xpos, (float)ypos));
        im.SetMousePositionChanged(true);
    }

    void windowSizeCallback(GLFWwindow* window, int width, int height)
    {
        // Keep the OS-level viewport covering the whole window
        ArkEngine::GetInstance().GetGraphicsAPI().SetViewport(0, 0, width, height);
    }

    void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
    }

    void charCallback(GLFWwindow* window, unsigned int codepoint)
    {
        if (ImGui::GetCurrentContext())
            ImGui_ImplGlfw_CharCallback(window, codepoint);
    }

    // ── Singleton ────────────────────────────────────────────────────────

    ArkEngine& ArkEngine::GetInstance()
    {
        static ArkEngine instance;
        return instance;
    }

    // ── Init ─────────────────────────────────────────────────────────────

    bool ArkEngine::Init(int width, int height)
    {
        if (!m_application) return false;

        Scene::RegisterTypes();
        m_application->RegisterTypes();

#if defined(__linux__)
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

        if (!glfwInit()) return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, "Ark Engine", nullptr, nullptr);
        if (!m_window)
        {
            std::cout << "Error creating window\n";
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(m_window, keyCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetCursorPosCallback(m_window, cursorPositionCallback);
        glfwSetWindowSizeCallback(m_window, windowSizeCallback);
        glfwSetScrollCallback(m_window, scrollCallback);
        glfwSetCharCallback(m_window, charCallback);

        glfwMakeContextCurrent(m_window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD\n";
            return false;
        }

        m_graphicsAPI.Init();
        m_graphicsAPI.SetViewport(0, 0, width, height);
        m_physicsManager.Init();
        m_audioManager.Init();
        m_renderQueue.Init();
        m_deferredRenderer.Init(width, height);
        m_fontManager.Init();

        // Create the scene FBO at the initial window size
        CreateFBO(width, height);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        const bool glfwReady = ImGui_ImplGlfw_InitForOpenGL(m_window, false);
        const bool glReady = glfwReady && ImGui_ImplOpenGL3_Init("#version 330");
        m_editorUIActive = glfwReady && glReady;

        if (!m_editorUIActive)
        {
            if (glfwReady) ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        return m_application->Init();
    }

    // ── Run ──────────────────────────────────────────────────────────────

    void ArkEngine::Run()
    {
        if (!m_application) return;

        m_lastTimePoint = std::chrono::steady_clock::now();

        while (!glfwWindowShouldClose(m_window) && !m_application->NeedsToBeClosed())
        {
            glfwPollEvents();

            auto  now = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
            m_lastTimePoint = now;

            m_physicsManager.Update(deltaTime);

            if (m_uiInputSystem.IsActive())
                m_uiInputSystem.Update(deltaTime);

            m_application->Update(deltaTime);

            // ── 1. Render scene into FBO ─────────────────────────
            int winW = 0, winH = 0;
            glfwGetWindowSize(m_window, &winW, &winH);

            // Resize FBO if window changed
            if (winW != m_fboWidth || winH != m_fboHeight)
                ResizeFBO(winW, winH);

            CameraData cameraData;
            std::vector<LightData> lights;
            float aspect = (m_fboHeight > 0)
                ? (float)m_fboWidth / (float)m_fboHeight : 1.0f;

            if (m_currentScene)
            {
                if (auto cam = m_currentScene->GetMainCamera())
                {
                    if (auto cc = cam->GetComponent<CameraComponent>())
                    {
                        cameraData.viewMatrix = cc->GetViewMatrix();
                        cameraData.projectionMatrix = cc->GetProjectionMatrix(aspect);
                        cameraData.orthoMatrix = glm::ortho(
                            0.0f, (float)m_fboWidth, 0.0f, (float)m_fboHeight);
                        cameraData.position = cam->GetWorldPosition();
                    }
                }
                lights = m_currentScene->CollectLights();
            }

            const bool useDeferred = m_deferredRenderingEnabled && m_deferredRenderer.GetOutputTexture() != 0;
            if (useDeferred)
            {
                m_deferredRenderer.Render(
                    m_renderQueue.FlushCommands(),
                    cameraData,
                    lights,
                    m_shadowStrength);

                glBindFramebuffer(GL_FRAMEBUFFER, m_deferredRenderer.GetOutputFramebuffer());
                m_graphicsAPI.SetViewport(0, 0, m_fboWidth, m_fboHeight);
                m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
                m_graphicsAPI.SetViewport(0, 0, m_fboWidth, m_fboHeight);
                m_graphicsAPI.ClearBuffers();
                m_renderQueue.Draw(m_graphicsAPI, cameraData, lights);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ── 2. Clear the default framebuffer ──
            m_graphicsAPI.SetViewport(0, 0, winW, winH);
            m_graphicsAPI.ClearBuffers();

            // ── 2b. Blit FBO to screen when editor viewport is not showing it ──
            if (!m_editorViewportActive)
            {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, useDeferred ? m_deferredRenderer.GetOutputFramebuffer() : m_fbo);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBlitFramebuffer(
                    0, 0, m_fboWidth, m_fboHeight,
                    0, 0, winW, winH,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // ── 3. ImGui ─────────────────────────────────────────
            if (m_editorUIActive)
            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                m_application->RenderUI();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            glfwSwapBuffers(m_window);
            m_inputManager.ClearStates();
        }

        m_application.reset(nullptr);
    }

    // ── Destroy ───────────────────────────────────────────────────────────

    void ArkEngine::Destroy()
    {
        if (m_application)
        {
            m_application->Destroy();
            m_application.reset();
        }

        DestroyFBO();

        if (m_editorUIActive)
            ImGui_ImplOpenGL3_Shutdown();

        glfwTerminate();
        m_window = nullptr;
    }

    // ── FBO helpers ───────────────────────────────────────────────────────

    void ArkEngine::CreateFBO(int width, int height)
    {
        m_fboWidth = width;
        m_fboHeight = height;

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // Colour attachment
        glGenTextures(1, &m_fboColorTex);
        glBindTexture(GL_TEXTURE_2D, m_fboColorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboColorTex, 0);

        // Depth + stencil renderbuffer
        glGenRenderbuffers(1, &m_fboDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, m_fboDepthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ArkEngine: FBO incomplete\n";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ArkEngine::ResizeFBO(int width, int height)
    {
        if (width <= 0 || height <= 0) return;
        DestroyFBO();
        CreateFBO(width, height);
        m_deferredRenderer.Resize(width, height);
    }

    void ArkEngine::DestroyFBO()
    {
        m_deferredRenderer.Shutdown();
        if (m_fbo) { glDeleteFramebuffers(1, &m_fbo);         m_fbo = 0; }
        if (m_fboColorTex) { glDeleteTextures(1, &m_fboColorTex); m_fboColorTex = 0; }
        if (m_fboDepthRBO) { glDeleteRenderbuffers(1, &m_fboDepthRBO); m_fboDepthRBO = 0; }
    }

    // ── Accessors ─────────────────────────────────────────────────────────

    void ArkEngine::SetCursorEnabled(bool enabled)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR,
            enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    bool ArkEngine::IsEditorUIActive() const { return m_editorUIActive; }

    void ArkEngine::SetApplication(Application* app) { m_application.reset(app); }
    Application* ArkEngine::GetApplication() { return m_application.get(); }

    InputManager& ArkEngine::GetInputManager() { return m_inputManager; }
    GraphicsAPI& ArkEngine::GetGraphicsAPI() { return m_graphicsAPI; }
    RenderQueue& ArkEngine::GetRenderQueue() { return m_renderQueue; }
    FileSystem& ArkEngine::GetFileSystem() { return m_fileSystem; }
    TextureManager& ArkEngine::GetTextureManager() { return m_textureManager; }
    MeshManager& ArkEngine::GetMeshManager() { return m_meshManager; }
    PhysicsManager& ArkEngine::GetPhysicsManager() { return m_physicsManager; }
    AudioManager& ArkEngine::GetAudioManager() { return m_audioManager; }
    FontManager& ArkEngine::GetFontManager() { return m_fontManager; }
    UIInputSystem& ArkEngine::GetUIInputSystem() { return m_uiInputSystem; }

    void ArkEngine::SetScene(const std::shared_ptr<Scene>& scene) { m_currentScene = scene; }
    const std::shared_ptr<Scene>& ArkEngine::GetScene() { return m_currentScene; }

    GLuint ArkEngine::GetSceneColorTexture() const
    {
        return m_deferredRenderingEnabled && m_deferredRenderer.GetOutputTexture() != 0
            ? m_deferredRenderer.GetOutputTexture()
            : m_fboColorTex;
    }

    void ArkEngine::SetDeferredRenderingEnabled(bool enabled)
    {
        m_deferredRenderingEnabled = enabled;
    }

    bool ArkEngine::IsDeferredRenderingEnabled() const
    {
        return m_deferredRenderingEnabled;
    }

    void ArkEngine::SetShadowStrength(float strength)
    {
        m_shadowStrength = std::clamp(strength, 0.0f, 1.0f);
    }

    float ArkEngine::GetShadowStrength() const
    {
        return m_shadowStrength;
    }
}