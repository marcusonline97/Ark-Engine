#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ArkApp.h"
#include "ArkWindow.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>



#include "Logger.h"
#include "Utility/Utility.h"
#include "Input/Input.h"

static constexpr const char* kImGuiGLSLVersion = "#version 450";

GLFWwindow* App::GetWindowHandle() const
{
    return m_Window ? m_Window->GetNativeHandle() : nullptr;
}

App::App()
{
    m_Window = std::make_unique<ArkWindow>(1400, 840, "Ark Engine");
    Ark::Input::SetWindow(m_Window->GetNativeHandle());

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(
        m_Window->GetNativeHandle(),
        [](GLFWwindow*, int width, int height)
        {
            glViewport(0, 0, width, height);
        }
    );

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    const bool imguiOk = m_ImGui.Init(m_Window->GetNativeHandle(), kImGuiGLSLVersion);
    Logging::ToDo() << "Initializing ImGui.\n";

    if (imguiOk)
    {
        m_EditorUI.Init();
        m_LogSinkId = Logging::AddSink([this](Logging::Level lvl, std::string_view msg)
            {
                m_EditorUI.PushLog(lvl, msg);
            });
    }
    {
        EditorObject obj{};
        obj.name = "Static Mesh";
        m_Objects.push_back(obj);        m_Objects.back().staticMesh = StaticMeshEditorComponent{};
        m_Objects.back().tint = glm::vec3(0.95f, 0.95f, 0.95f);

        obj = EditorObject{};
        obj.name = "Skeletal Mesh";
        m_Objects.push_back(obj);        m_Objects.back().skeletalMesh = SkeletalMeshEditorComponent{};
        m_Objects.back().position = glm::vec3(1.5f, 0.0f, 0.0f);
        m_Objects.back().tint = glm::vec3(0.65f, 0.85f, 1.0f);

        obj = EditorObject{};
        obj.name = "Camera";
        m_Objects.push_back(obj);        m_Objects.back().camera = CameraEditorComponent{};
        m_Objects.back().position = glm::vec3(0.0f, 0.0f, 3.0f);

        obj = EditorObject{};
        obj.name = "Point Light";
        m_Objects.push_back(obj);        m_Objects.back().pointLight = PointLightEditorComponent{};
        m_Objects.back().position = glm::vec3(-1.25f, 1.0f, 0.0f);
        m_Objects.back().scale = glm::vec3(0.2f);
    }
    m_SelectedObject = 0;

    m_WorldRenderer = std::make_unique<Ark::Rendering::WorldRenderThread>(m_Window->GetNativeHandle());
}

App::~App()
{
    if (m_LogSinkId != 0)
    {
        Logging::RemoveSink(m_LogSinkId);
        m_LogSinkId = 0;
    }

    m_EditorUI.Shutdown();
    m_ImGui.Shutdown();
    m_WorldRenderer.reset();
}


void App::Run()
{
    double lastTime = glfwGetTime();
    bool rotating = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    while (!m_Window->ShouldClose())
    {
            const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        Ark::Input::NewFrame();

        // Play mode: possess the primary camera and drive it with basic FPS controls.
        if (m_EditorUI.IsPlayMode())
        {
                EditorObject* camObj = nullptr;
            for (auto& o : m_Objects)
            {
                if (!o.enabled || !o.camera) continue;
                if (o.camera->primary) { camObj = &o; break; }
            }
            if (!camObj)
            {
                for (auto& o : m_Objects)
                {
                    if (!o.enabled || !o.camera) continue;
                    camObj = &o;
                    break;
                }
            }

            if (camObj)
            {
                const float pitch = camObj->rotationDeg.x;
                const float yaw = camObj->rotationDeg.y;

                glm::vec3 front;
                front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
                front.y = sin(glm::radians(pitch));
                front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
                front = glm::normalize(front);

                const glm::vec3 up(0.0f, 1.0f, 0.0f);
                const glm::vec3 right = glm::normalize(glm::cross(front, up));

                const float speed = 3.5f;
                const float move = speed * dt;

                if (Ark::Input::IsKeyDown(ARK_KEY_W)) camObj->position += front * move;
                if (Ark::Input::IsKeyDown(ARK_KEY_S)) camObj->position -= front * move;
                if (Ark::Input::IsKeyDown(ARK_KEY_D)) camObj->position += right * move;
                if (Ark::Input::IsKeyDown(ARK_KEY_A)) camObj->position -= right * move;
                if (Ark::Input::IsKeyDown(ARK_KEY_E)) camObj->position += up * move;
                if (Ark::Input::IsKeyDown(ARK_KEY_Q)) camObj->position -= up * move;

                // Hold RMB to rotate the camera.
                if (Ark::Input::IsMouseDown(ARK_MOUSE_RIGHT))
                {
                    double mx = 0.0, my = 0.0;
                    glfwGetCursorPos(m_Window->GetNativeHandle(), &mx, &my);
                    if (!rotating)
                    {
                        rotating = true;
                        lastMouseX = mx;
                        lastMouseY = my;
                    }
                    else
                    {
                        const double dx = mx - lastMouseX;
                        const double dy = my - lastMouseY;
                        lastMouseX = mx;
                        lastMouseY = my;

                        constexpr float sensitivity = 0.12f;
                        camObj->rotationDeg.y += static_cast<float>(dx) * sensitivity;
                        camObj->rotationDeg.x -= static_cast<float>(dy) * sensitivity;

                        // Clamp pitch to avoid gimbal flip.
                        if (camObj->rotationDeg.x > 89.0f) camObj->rotationDeg.x = 89.0f;
                        if (camObj->rotationDeg.x < -89.0f) camObj->rotationDeg.x = -89.0f;
                    }
                }
                else
                {
                    rotating = false;
                }
            }
        }

        glm::vec2 vpSize = m_EditorUI.GetViewportSize();
        int vpW = static_cast<int>(vpSize.x);
        int vpH = static_cast<int>(vpSize.y);
        if (vpW < 16 || vpH < 16)
        {
            // Fallback to window size until the viewport panel is laid out.
            glfwGetFramebufferSize(m_Window->GetNativeHandle(), &vpW, &vpH);
        }
        if (vpW < 1) vpW = 1;
        if (vpH < 1) vpH = 1;

        if (m_WorldRenderer)
        {
            Ark::Rendering::WorldRenderInput input{};
            input.width = static_cast<uint32_t>(vpW);
            input.height = static_cast<uint32_t>(vpH);

                const auto toModel = [](const EditorObject& obj)
                {
                    const glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
                    const glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
                    const glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
                    const glm::mat4 rot = rotZ * rotY * rotX;
                    return glm::translate(glm::mat4(1.0f), obj.position) * rot * glm::scale(glm::mat4(1.0f), obj.scale);
                };

            // Camera selection: primary camera wins, otherwise first camera.
            {
                EditorObject* camObj = nullptr;
                for (auto& o : m_Objects)
                {
                    if (!o.enabled || !o.camera) continue;
                    if (o.camera->primary) { camObj = &o; break; }
                    if (!camObj) camObj = &o;
                }

                if (camObj && camObj->camera)
                {
                    input.camera.position = camObj->position;
                    input.camera.pitchYawDeg = glm::vec2(camObj->rotationDeg.x, camObj->rotationDeg.y);
                    input.camera.fovDeg = camObj->camera->fovDeg;
                    input.camera.nearPlane = camObj->camera->nearPlane;
                    input.camera.farPlane = camObj->camera->farPlane;
                }
            }

            // Render instances: static/skeletal meshes (as proxy cubes for now) + point lights (small proxy cubes).
            input.instances.clear();
            input.instances.reserve(m_Objects.size());

            for (const auto& o : m_Objects)
            {
                if (!o.enabled)
                    continue;

                if (o.staticMesh || o.skeletalMesh)
                {
                    Ark::Rendering::RenderInstance inst{};
                    inst.model = toModel(o);
                    inst.tint = o.tint;
                    if (o.staticMesh && !o.staticMesh->meshPath.empty())
                        inst.meshPath = o.staticMesh->meshPath;
                    else if (o.skeletalMesh && !o.skeletalMesh->meshPath.empty())
                        inst.meshPath = o.skeletalMesh->meshPath;
                    input.instances.push_back(inst);
                }

                if (o.pointLight)
                {
                    Ark::Rendering::RenderInstance inst{};

                    EditorObject proxy = o;
                    // Keep the light proxy small-ish while still reflecting radius a bit.
                    const float r = o.pointLight->radius;
                    const float s = std::max(0.05f, 0.10f * r);
                    proxy.scale = glm::vec3(s);

                    inst.model = toModel(proxy);

                    const glm::vec3 raw = o.pointLight->color * o.pointLight->intensity;
                    inst.tint = glm::clamp(raw, glm::vec3(0.0f), glm::vec3(1.0f));
                    input.instances.push_back(inst);
                }
            }

            m_WorldRenderer->Submit(input);
        }

        Utilities::TickViewportFPS(glfwGetTime());


        // Clear the main framebuffer before drawing UI.
        int winW = 0, winH = 0;
        glfwGetFramebufferSize(m_Window->GetNativeHandle(), &winW, &winH);
        glViewport(0, 0, winW, winH);
        glClearColor(0.12f, 0.12f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_ImGui.IsInitialized())
        {
            m_ImGui.BeginFrame();
            m_EditorUI.SetViewportTextureId(m_WorldRenderer ? m_WorldRenderer->GetLatestTextureId() : 0);            m_EditorUI.Render(m_Objects, m_SelectedObject);
            m_ImGui.EndFrame();
        }

        m_Window->SwapBuffers();
        m_Window->PollEvents();
    }
}